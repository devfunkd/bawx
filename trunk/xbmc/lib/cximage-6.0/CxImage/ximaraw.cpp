/*
 * File:	ximaraw.cpp
 * Purpose:	Platform Independent RAW Image Class Loader
 * 16/Dec/2007 Davide Pizzolato - www.xdp.it
 * CxImage version 6.0.0 02/Feb/2008
 * 
 * CxImageRAW (c) May/2006 pdw63
 *
 * based on dcraw.c -- Dave Coffin's raw photo decoder
 * Copyright 1997-2007 by Dave Coffin, dcoffin a cybercom o net
 */

#ifdef _LINUX
#include <pthread.h>
#else
#include <windows.h>
#endif
#include <setjmp.h>
#include "ximaraw.h"

#if CXIMAGE_SUPPORT_RAW

#ifdef _LINUX
static pthread_mutex_t rawMutex;
#else
static HANDLE rawMutex;
#endif
static bool mutexInitialized = false; 

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
bool CxImageRAW::Decode(CxFile *hFile)
{
return false;
#if 0
	if (hFile==NULL)
		return false;

	if (mutexInitialized == false)
	{
#ifdef _LINUX
		pthread_mutex_init(&rawMutex, 0);
#else
		rawMutex = ::CreateMutex(NULL, false, NULL);
#endif
		mutexInitialized = true;
	}

#ifdef _LINUX
	pthread_mutex_lock(&rawMutex);
#else
	::WaitForSingleObject(rawMutex, INFINITE);
#endif
 
	DCRAW  dcr;

  cx_try
  {
	// initialization
	dcr_init_dcraw(&dcr);

	dcr.opt.user_qual = GetCodecOption(CXIMAGE_FORMAT_RAW) & 0x03;

	// setup variables for debugging
	char szClass[] = "CxImageRAW";
	dcr.ifname = szClass;
	dcr.sz_error = info.szLastError;

	// setup library options, see dcr_print_manual for the available switches
	// call dcr_parse_command_line_options(&dcr,0,0,0) to set default options
	// if (dcr_parse_command_line_options(&dcr,argc,argv,&arg))
	if (dcr_parse_command_line_options(&dcr,0,0,0)){
		cx_throw("CxImageRAW: unknown option");
	}

	// set return point for error handling
	if (setjmp (dcr.failure)) {
		cx_throw("");
	}

	// install file manager
	CxFileRaw src(hFile,&dcr);

	// check file header
	dcr_identify(&dcr);
	
	if(!dcr.is_raw){
		cx_throw("CxImageRAW: not a raw image");
	}

	if (dcr.load_raw == NULL) {
		cx_throw("CxImageRAW: missing raw decoder");
	}

	// verify special case
	if (dcr.load_raw == dcr_kodak_ycbcr_load_raw) {
		dcr.height += dcr.height & 1;
		dcr.width  += dcr.width  & 1;
	}

	if (info.nEscape == -1){
		head.biWidth = dcr.width;
		head.biHeight= dcr.height;
		info.dwType = CXIMAGE_FORMAT_RAW;
		cx_throw("output dimensions returned");
	}

	// shrinked decoding available and requested?
	dcr.shrink = dcr.filters && (dcr.opt.half_size || dcr.opt.threshold || dcr.opt.aber[0] != 1 || dcr.opt.aber[2] != 1);
	dcr.iheight = (dcr.height + dcr.shrink) >> dcr.shrink;
	dcr.iwidth  = (dcr.width  + dcr.shrink) >> dcr.shrink;

	// install custom camera matrix
	if (dcr.opt.use_camera_matrix && dcr.cmatrix[0][0] > 0.25) {
		memcpy (dcr.rgb_cam, dcr.cmatrix, sizeof dcr.cmatrix);
		dcr.raw_color = 0;
	}
    else {
      dcr.opt.use_camera_wb = 1;
    }

	// allocate memory for the image
	dcr.image = (ushort (*)[4]) calloc (dcr.iheight*dcr.iwidth, sizeof *dcr.image);
	dcr_merror (&dcr, dcr.image, szClass);

	if (dcr.meta_length) {
		dcr.meta_data = (char *) malloc (dcr.meta_length);
		dcr_merror (&dcr, dcr.meta_data, szClass);
	}

	// start image decoder
	hFile->Seek(dcr.data_offset, SEEK_SET);
	(*dcr.load_raw)(&dcr);

	// post processing
	if (dcr.zero_is_bad) dcr_remove_zeroes(&dcr);

	dcr_bad_pixels(&dcr,NULL);

	if (dcr.opt.dark_frame) dcr_subtract (&dcr,dcr.opt.dark_frame);

	dcr.quality = 2 + !dcr.fuji_width;

	if (dcr.opt.user_qual >= 0) dcr.quality = dcr.opt.user_qual;

	if (dcr.opt.user_black >= 0) dcr.black = dcr.opt.user_black;

#ifdef COLORCHECK
	dcr_colorcheck(&dcr);
#endif

#if RESTRICTED
	if (dcr.is_foveon && !dcr.opt.document_mode) dcr_foveon_interpolate(&dcr);
#endif

	if (!dcr.is_foveon && dcr.opt.document_mode < 2) dcr_scale_colors(&dcr);

	// pixel interpolation and filters
	dcr_pre_interpolate(&dcr);

	if (dcr.filters && !dcr.opt.document_mode) {
		if (dcr.quality == 0)
			dcr_lin_interpolate(&dcr);
		else if (dcr.quality == 1 || dcr.colors > 3)
			dcr_vng_interpolate(&dcr);
		else if (dcr.quality == 2)
			dcr_ppg_interpolate(&dcr);
		else
			dcr_ahd_interpolate(&dcr);
	}

	if (dcr.mix_green) {
		long i;
		for (dcr.colors=3, i=0; i < dcr.height*dcr.width; i++) {
			dcr.image[i][1] = (dcr.image[i][1] + dcr.image[i][3]) >> 1;
		}
	}

	if (!dcr.is_foveon && dcr.colors == 3) dcr_median_filter(&dcr);

	if (!dcr.is_foveon && dcr.opt.highlight == 2) dcr_blend_highlights(&dcr);

	if (!dcr.is_foveon && dcr.opt.highlight > 2) dcr_recover_highlights(&dcr);

	if (dcr.opt.use_fuji_rotate) dcr_fuji_rotate(&dcr);

#ifndef NO_LCMS
	if (dcr.opt.cam_profile) dcr_apply_profile (dcr.opt.cam_profile, dcr.opt.out_profile);
#endif

	// final conversion
	dcr_convert_to_rgb(&dcr);

	if (dcr.opt.use_fuji_rotate) dcr_stretch(&dcr);

	dcr.iheight = dcr.height;
	dcr.iwidth  = dcr.width;
	if (dcr.flip & 4) SWAP(dcr.height,dcr.width);

	// ready to transfer data from dcr.image
	if (!Create(dcr.width,dcr.height,24,CXIMAGE_FORMAT_RAW)){
		cx_throw("");
	}

	uchar  *ppm = (uchar *) calloc (dcr.width, dcr.colors*dcr.opt.output_bps/8);
	ushort *ppm2 = (ushort *) ppm;
	dcr_merror (&dcr, ppm, szClass);

	uchar lut[0x10000];
	if (dcr.opt.output_bps == 8) dcr_gamma_lut (&dcr, lut);

	long c, row, col, soff, rstep, cstep;
	soff  = dcr_flip_index (&dcr, 0, 0);
	cstep = dcr_flip_index (&dcr, 0, 1) - soff;
	rstep = dcr_flip_index (&dcr, 1, 0) - dcr_flip_index (&dcr, 0, dcr.width);
	for (row=0; row < dcr.height; row++, soff += rstep) {
		for (col=0; col < dcr.width; col++, soff += cstep) {
			if (dcr.opt.output_bps == 8)
				for (c=0; c < dcr.colors; c++) ppm [col*dcr.colors+c] = lut[dcr.image[soff][c]];
			else
				for (c=0; c < dcr.colors; c++) ppm2[col*dcr.colors+c] = dcr.image[soff][c];
		}
		if (dcr.opt.output_bps == 16 && !dcr.opt.output_tiff && htons(0x55aa) != 0x55aa)
#if defined(_LINUX) || defined(__APPLE__)
			swab ((char*)ppm2, (char*)ppm2, dcr.width*dcr.colors*2);
#else
			_swab ((char*)ppm2, (char*)ppm2, dcr.width*dcr.colors*2);
#endif

		DWORD size = dcr.width * (dcr.colors*dcr.opt.output_bps/8);
		RGBtoBGR(ppm,size);
		memcpy(GetBits(dcr.height - 1 - row), ppm, min(size,GetEffWidth()));
	}
	free (ppm);


	dcr_cleanup_dcraw(&dcr);
#ifdef _LINUX
	pthread_mutex_unlock(&rawMutex);
#else
	::ReleaseMutex(rawMutex);
#endif

  } cx_catch {

	dcr_cleanup_dcraw(&dcr);
#ifdef _LINUX
	pthread_mutex_unlock(&rawMutex);
#else
	::ReleaseMutex(rawMutex);
#endif

	if (strcmp(message,"")) strncpy(info.szLastError,message,255);
	if (info.nEscape == -1 && info.dwType == CXIMAGE_FORMAT_RAW) return true;
	return false;
  }
	/* that's it */
	return true;
#endif
}

#ifdef XBMC
bool CxImageRAW::GetExifThumbnail(const char *filename, const char *outname, int type)
{	
	DCRAW  dcr;

	CxIOFile file;
	if (!file.Open(filename, "rb")) 
		return false;

  cx_try
  {
	// initialization
	dcr_init_dcraw(&dcr);

	dcr.opt.user_qual = GetCodecOption(CXIMAGE_FORMAT_RAW) & 0x03;

	// setup variables for debugging
	char szClass[] = "CxImageRAW";
	dcr.ifname = szClass;
	dcr.sz_error = info.szLastError;

	// setup library options, see dcr_print_manual for the available switches
	// call dcr_parse_command_line_options(&dcr,0,0,0) to set default options
	// if (dcr_parse_command_line_options(&dcr,argc,argv,&arg))
	if (dcr_parse_command_line_options(&dcr,0,0,0)){
		cx_throw("CxImageRAW: unknown option");
	}

	// set return point for error handling
	if (setjmp (dcr.failure)) {
		cx_throw("");
	}

	// install file manager
	CxFileRaw src(&file,&dcr);

	// check file header
	dcr_identify(&dcr);

	if(!dcr.is_raw){
		cx_throw("CxImageRAW: not a raw image");
	}

	if (dcr.load_raw == NULL) {
		cx_throw("CxImageRAW: missing raw decoder");
	}
	
	// THUMB.
	if (dcr.thumb_offset != 0)
	{
 		FILE* file = fopen(outname, "wb");
		DCRAW* p = &dcr;
		dcr_fseek(dcr.obj_, dcr.thumb_offset, SEEK_SET);
		dcr.write_thumb(&dcr, file);
		fclose(file);
		
		// Read in the thumbnail to resize and rotate.
		CxImage image(outname, CXIMAGE_FORMAT_UNKNOWN);
	    if (image.IsValid())
	    {
			// Resizing.
      		if (image.GetWidth() > 256 || image.GetHeight() > 256)
		    {
				float amount = 256.0f / max(image.GetWidth(), image.GetHeight());
				image.Resample((long)(image.GetWidth() * amount), (long)(image.GetHeight() * amount), 0);
		    }
	      	
			// Rotation.
			if (p->flip != 0)
		    	image.RotateExif(p->flip);
		    
			return image.Save(outname, CXIMAGE_FORMAT_JPG);
		}
	}
	else
	{
		cx_throw("No thumbnail!");
	}
	
	dcr_cleanup_dcraw(&dcr);

  } cx_catch {

	dcr_cleanup_dcraw(&dcr);

	if (strcmp(message,"")) strncpy(info.szLastError,message,255);
	if (info.nEscape == -1 && info.dwType == CXIMAGE_FORMAT_RAW) return true;
	return false;
  }
	/* that's it */
	return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
bool CxImageRAW::Encode(CxFile * hFile)
{
	if (hFile == NULL) return false;
	strcpy(info.szLastError, "Save RAW not supported");
	return false;
}
////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_RAW

