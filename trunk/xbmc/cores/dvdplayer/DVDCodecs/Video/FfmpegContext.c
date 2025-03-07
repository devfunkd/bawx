

/* 
 * $Id: FfmpegContext.c 1308 2009-10-25 13:21:21Z casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAS_DX

//#define HAVE_AV_CONFIG_H
#define H264_MERGE_TESTING

// start boxee
#define __asm__ __asm
#define inline __inline
// end boxee

#include <windows.h>
#include <winnt.h>
#include <vfwmsgs.h>
#include "FfmpegContext.h"
#include "dsputil.h"
#include "avcodec.h"
#include "mpegvideo.h"
#include "golomb.h"

#include "h264.h"
#include "h264data.h"
#include "vc1.h"

// boxee start
#undef BYTE
typedef unsigned char BYTE;
// boxee end


int av_h264_decode_frame(struct AVCodecContext* avctx, uint8_t *buf, int buf_size);
int av_vc1_decode_frame(struct AVCodecContext *avctx, uint8_t *buf, int buf_size);


const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

int IsVistaAndAbove()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
	if (	GetVersionEx( &osver ) && 
			osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
			(osver.dwMajorVersion >= 6 ) )
		return 1;

	return 0;
}

char* GetFFMpegPictureType(int nType)
{
	static char*	s_FFMpegPictTypes[] = { "? ", "I ", "P ", "B ", "S ", "SI", "SP" };
	int		nTypeCount = sizeof(s_FFMpegPictTypes)/sizeof(TCHAR)-1;

	return s_FFMpegPictTypes[min(nType, nTypeCount)];
}


void FFH264DecodeBuffer (struct AVCodecContext* pAVCtx, BYTE* pBuffer, UINT nSize, int* pFramePOC, int* pOutPOC, REFERENCE_TIME* pOutrtStart)
{
	if (pBuffer != NULL)
	{
		H264Context*	h	= (H264Context*) pAVCtx->priv_data;
		av_h264_decode_frame (pAVCtx, pBuffer, nSize);

		if (h->s.current_picture_ptr  && pFramePOC) *pFramePOC = h->s.current_picture_ptr->field_poc[0];

		if (pOutPOC)		*pOutPOC		= h->outputed_poc;
		if (pOutrtStart)	*pOutrtStart	= h->outputed_rtstart;
	}
}


int FFH264CheckCompatibility(int nWidth, int nHeight, struct AVCodecContext* pAVCtx, BYTE* pBuffer, UINT nSize, int nPCIVendor, LARGE_INTEGER VideoDriverVersion)
{
	H264Context*	pContext	= (H264Context*) pAVCtx->priv_data;
	SPS*			cur_sps;
	PPS*			cur_pps;

	int supportLevel51 = 0;

// BOXEE begin
// Patch the avc flags
  int is_avc;
  if(pAVCtx->extradata_size > 0 && pAVCtx->extradata &&
    (*(char *)pAVCtx->extradata == 1 || (pAVCtx->codec_tag == 0x31637661 || pAVCtx->codec_tag == 0x31435641))){
      is_avc = 1;
  } else {
    is_avc = 0;
  }
  pContext->is_avc = is_avc;
  pContext->got_avcC = 0;
// BOXEE end

	if (pBuffer != NULL)
		av_h264_decode_frame (pAVCtx, pBuffer, nSize);

	cur_sps		= pContext->sps_buffers[0];
	cur_pps		= pContext->pps_buffers[0];

	if (cur_sps != NULL)
	{
		
		if (nPCIVendor == PCIV_nVidia) {
			// nVidia cards support level 5.1 since drivers v6.14.11.7800 for XP and drivers v7.15.11.7800 for Vista
			// vA.B.C.D
			int A, B, C, D;
			if (IsVistaAndAbove()) {
				A = 7; B = 15; C = 11; D = 7800;
			} else {
				A = 6; B = 14; C = 11; D = 7800;
			}

			if (HIWORD(VideoDriverVersion.HighPart) > A) {
				supportLevel51 = 1;
			} else if (HIWORD(VideoDriverVersion.HighPart) == A) {
				if (LOWORD(VideoDriverVersion.HighPart) > B) {
					supportLevel51 = 1;
				} else if (LOWORD(VideoDriverVersion.HighPart) == B) {
					if (HIWORD(VideoDriverVersion.LowPart) > C) {
						supportLevel51 = 1;
					} else if (HIWORD(VideoDriverVersion.LowPart) == C) {
						if (LOWORD(VideoDriverVersion.LowPart) >= D) {
							supportLevel51 = 1;
						}
					}
				}
			}
		}
		else if (nPCIVendor == PCIV_S3_Graphics)
			supportLevel51 = 1;

		// Check max num reference frame according to the level
		#define MAX_DPB_41 12288 // DPB value for level 4.1

		if (supportLevel51 == 1) {
			// 11 refs as absolute max, but for Nvidia(Vista, HD) - 16
			if(IsVistaAndAbove()) {
				if(nWidth>1279) {
					if (cur_sps->ref_frame_count > 16)
						return 2;	// Too much ref frames					
				} else {
					if (cur_sps->ref_frame_count > 11)
						return 2;	// Too much ref frames
				}
			} else {
				if (cur_sps->ref_frame_count > 14)
					return 2;	// Too much ref frames
			}
		} else {
			// level 4.1 with 11 refs as absolute max
			if (cur_sps->ref_frame_count > min(11, (1024*MAX_DPB_41/(nWidth*nHeight*1.5))))
				return 2;	// Too much ref frames
		}
	
	}
		
	return 0;
}

void FFH264GetMBSize(struct AVCodecContext* pAVCtx, int* mbWidth, int* mbHeight)
{
  H264Context*	pContext	= (H264Context*) pAVCtx->priv_data;
  SPS*			cur_sps;

  if(!mbWidth || !mbHeight)
    return;

  if(pContext)
    cur_sps = pContext->sps_buffers[0];
  if(cur_sps)
  {
    *mbWidth = cur_sps->mb_width;
    *mbHeight = cur_sps->mb_height;
  }
  else
  {
    *mbWidth = 0;
    *mbHeight = 0;
  }
}


void CopyScalingMatrix(DXVA_Qmatrix_H264* pDest, DXVA_Qmatrix_H264* pSource, int nPCIVendor)
{
	int		i,j;

	switch (nPCIVendor)
	{
	case PCIV_ATI :
		// The ATI way
		memcpy (pDest, pSource, sizeof (DXVA_Qmatrix_H264));
		break;

	default :
		// The nVidia way (and other manufacturers compliant with specifications....)
		for (i=0; i<6; i++)
			for (j=0; j<16; j++)
				pDest->bScalingLists4x4[i][j] = pSource->bScalingLists4x4[i][ZZ_SCAN[j]];

		for (i=0; i<2; i++)
			for (j=0; j<64; j++)
				pDest->bScalingLists8x8[i][j] = pSource->bScalingLists8x8[i][ZZ_SCAN8[j]];
		break;
	}
}

USHORT FFH264FindRefFrameIndex(USHORT num_frame, DXVA_PicParams_H264* pDXVAPicParams)
{
	int		i;
	for (i=0; i<pDXVAPicParams->num_ref_frames; i++)
	{
		if (pDXVAPicParams->FrameNumList[i] == num_frame)
			return pDXVAPicParams->RefFrameList[i].Index7Bits;
	}

#ifdef _DEBUG
//	DebugBreak();		// Ref frame not found !
#endif

	return 127;
}

HRESULT FFH264BuildPicParams (DXVA_PicParams_H264* pDXVAPicParams, DXVA_Qmatrix_H264* pDXVAScalingMatrix, int* nFieldType, int* nSliceType, struct AVCodecContext* pAVCtx, int nPCIVendor)
{
	H264Context*			h			= (H264Context*) pAVCtx->priv_data;
	SPS*					cur_sps;
	PPS*					cur_pps;
    MpegEncContext* const	s = &h->s;
	int						field_pic_flag;
	HRESULT					hr = E_FAIL;

	field_pic_flag = (h->s.picture_structure != PICT_FRAME);

	cur_sps	= &h->sps;
	cur_pps = &h->pps;

	if (cur_sps && cur_pps)
	{
		*nFieldType = h->s.picture_structure;
		if (h->sps.pic_struct_present_flag)
		{
            switch (h->sei_pic_struct)
            {
            case SEI_PIC_STRUCT_TOP_FIELD:
            case SEI_PIC_STRUCT_TOP_BOTTOM:
            case SEI_PIC_STRUCT_TOP_BOTTOM_TOP:
				*nFieldType = PICT_TOP_FIELD;
                break;
            case SEI_PIC_STRUCT_BOTTOM_FIELD:
            case SEI_PIC_STRUCT_BOTTOM_TOP:
            case SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM:
				*nFieldType = PICT_BOTTOM_FIELD;
                break;
            case SEI_PIC_STRUCT_FRAME_DOUBLING:
            case SEI_PIC_STRUCT_FRAME_TRIPLING:
            case SEI_PIC_STRUCT_FRAME:
				*nFieldType = PICT_FRAME;
                break;
			}
		}

		*nSliceType = h->slice_type;

		if (cur_sps->mb_width==0 || cur_sps->mb_height==0) return VFW_E_INVALID_FILE_FORMAT;
		pDXVAPicParams->wFrameWidthInMbsMinus1			= cur_sps->mb_width  - 1;		// pic_width_in_mbs_minus1;
		pDXVAPicParams->wFrameHeightInMbsMinus1			= cur_sps->mb_height * (2 - cur_sps->frame_mbs_only_flag) - 1;		// pic_height_in_map_units_minus1;
		pDXVAPicParams->num_ref_frames					= cur_sps->ref_frame_count;		// num_ref_frames;
		pDXVAPicParams->field_pic_flag					= field_pic_flag;
		pDXVAPicParams->MbaffFrameFlag					= (h->sps.mb_aff && (field_pic_flag==0));
		pDXVAPicParams->residual_colour_transform_flag	= cur_sps->residual_color_transform_flag;
		pDXVAPicParams->sp_for_switch_flag				= h->sp_for_switch_flag;
		pDXVAPicParams->chroma_format_idc				= cur_sps->chroma_format_idc;
		pDXVAPicParams->RefPicFlag						= h->ref_pic_flag;
		pDXVAPicParams->constrained_intra_pred_flag		= cur_pps->constrained_intra_pred;
		pDXVAPicParams->weighted_pred_flag				= cur_pps->weighted_pred;
		pDXVAPicParams->weighted_bipred_idc				= cur_pps->weighted_bipred_idc;
		pDXVAPicParams->frame_mbs_only_flag				= cur_sps->frame_mbs_only_flag;
		pDXVAPicParams->transform_8x8_mode_flag			= cur_pps->transform_8x8_mode;
		pDXVAPicParams->IntraPicFlag					= (h->slice_type == FF_I_TYPE );

		pDXVAPicParams->bit_depth_luma_minus8			= cur_sps->bit_depth_luma   - 8;	// bit_depth_luma_minus8
		pDXVAPicParams->bit_depth_chroma_minus8			= cur_sps->bit_depth_chroma - 8;	// bit_depth_chroma_minus8
	//	pDXVAPicParams->StatusReportFeedbackNumber		= SET IN DecodeFrame;


	//	pDXVAPicParams->CurrFieldOrderCnt						= SET IN UpdateRefFramesList;
	//	pDXVAPicParams->FieldOrderCntList						= SET IN UpdateRefFramesList;
	//	pDXVAPicParams->FrameNumList							= SET IN UpdateRefFramesList;
	//	pDXVAPicParams->UsedForReferenceFlags					= SET IN UpdateRefFramesList;
	//	pDXVAPicParams->NonExistingFrameFlags
		pDXVAPicParams->frame_num						= h->frame_num;
	//	pDXVAPicParams->SliceGroupMap


		pDXVAPicParams->log2_max_frame_num_minus4				= cur_sps->log2_max_frame_num - 4;					// log2_max_frame_num_minus4;
		pDXVAPicParams->pic_order_cnt_type						= cur_sps->poc_type;								// pic_order_cnt_type;
		pDXVAPicParams->log2_max_pic_order_cnt_lsb_minus4		= cur_sps->log2_max_poc_lsb - 4;					// log2_max_pic_order_cnt_lsb_minus4;
		pDXVAPicParams->delta_pic_order_always_zero_flag		= cur_sps->delta_pic_order_always_zero_flag;
		pDXVAPicParams->direct_8x8_inference_flag				= cur_sps->direct_8x8_inference_flag;
		pDXVAPicParams->entropy_coding_mode_flag				= cur_pps->cabac;									// entropy_coding_mode_flag;
		pDXVAPicParams->pic_order_present_flag					= cur_pps->pic_order_present;						// pic_order_present_flag;
		pDXVAPicParams->num_slice_groups_minus1					= cur_pps->slice_group_count - 1;					// num_slice_groups_minus1;
		pDXVAPicParams->slice_group_map_type					= cur_pps->mb_slice_group_map_type;					// slice_group_map_type;
		pDXVAPicParams->deblocking_filter_control_present_flag	= cur_pps->deblocking_filter_parameters_present;	// deblocking_filter_control_present_flag;
		pDXVAPicParams->redundant_pic_cnt_present_flag			= cur_pps->redundant_pic_cnt_present;				// redundant_pic_cnt_present_flag;
		pDXVAPicParams->slice_group_change_rate_minus1			= cur_pps->slice_group_change_rate_minus1;

		pDXVAPicParams->chroma_qp_index_offset					= cur_pps->chroma_qp_index_offset[0];
		pDXVAPicParams->second_chroma_qp_index_offset			= cur_pps->chroma_qp_index_offset[1];
		pDXVAPicParams->num_ref_idx_l0_active_minus1			= cur_pps->ref_count[0]-1;							// num_ref_idx_l0_active_minus1;
		pDXVAPicParams->num_ref_idx_l1_active_minus1			= cur_pps->ref_count[1]-1;							// num_ref_idx_l1_active_minus1;
		pDXVAPicParams->pic_init_qp_minus26						= cur_pps->init_qp - 26;
		pDXVAPicParams->pic_init_qs_minus26						= cur_pps->init_qs - 26;

		if (field_pic_flag)
		{
			pDXVAPicParams->CurrPic.AssociatedFlag  = (h->s.picture_structure == PICT_BOTTOM_FIELD);

			if (pDXVAPicParams->CurrPic.AssociatedFlag)
			{
				// Bottom field
				pDXVAPicParams->CurrFieldOrderCnt[0] = 0;
				pDXVAPicParams->CurrFieldOrderCnt[1] = h->poc_lsb + h->poc_msb;
			}
			else
			{
				// Top field
				pDXVAPicParams->CurrFieldOrderCnt[0] = h->poc_lsb + h->poc_msb;
				pDXVAPicParams->CurrFieldOrderCnt[1] = 0;
			}
		}
		else
		{
			pDXVAPicParams->CurrPic.AssociatedFlag	= 0;
			pDXVAPicParams->CurrFieldOrderCnt[0]	= h->poc_lsb + h->poc_msb;
			pDXVAPicParams->CurrFieldOrderCnt[1]	= h->poc_lsb + h->poc_msb;
		}

		CopyScalingMatrix (pDXVAScalingMatrix, (DXVA_Qmatrix_H264*)cur_pps->scaling_matrix4, nPCIVendor);
		hr = S_OK;
	}

	return hr;
}


void FFH264SetCurrentPicture (int nIndex, DXVA_PicParams_H264* pDXVAPicParams, struct AVCodecContext* pAVCtx)
{
	H264Context*	h			= (H264Context*) pAVCtx->priv_data;

	pDXVAPicParams->CurrPic.Index7Bits	= nIndex;

	if (h->s.current_picture_ptr)
		h->s.current_picture_ptr->opaque = (void*)nIndex;
}


void FFH264UpdateRefFramesList (DXVA_PicParams_H264* pDXVAPicParams, struct AVCodecContext* pAVCtx)
{
	H264Context*	h			= (H264Context*) pAVCtx->priv_data;
	UINT			nUsedForReferenceFlags = 0;
	int				i;
	Picture*		pic;
	UCHAR			AssociatedFlag;

	for(i = 0; i < 16; i++)
	{
    if (i < h->short_ref_count)
		{
			// Short list reference frames
      pic	= h->short_ref[h->short_ref_count - i - 1];
			AssociatedFlag	= 0;
		}
    else if (i >= h->short_ref_count && i < h->long_ref_count)
		{
			// Long list reference frames
      pic	= h->short_ref[h->short_ref_count + h->long_ref_count - i - 1];
			AssociatedFlag	= 1;
		}
		else
			pic = NULL;


		if (pic != NULL)
		{
			pDXVAPicParams->FrameNumList[i] = pic->frame_num;

			if (pic->field_poc[0] != INT_MAX)
			{
				pDXVAPicParams->FieldOrderCntList[i][0] = pic->field_poc [0];
				nUsedForReferenceFlags |= 1<<(i*2);
			}
			else
				pDXVAPicParams->FieldOrderCntList[i][0]		= 0;

			if (pic->field_poc[1] != INT_MAX)
			{
				pDXVAPicParams->FieldOrderCntList[i][1]		= pic->field_poc [1];
				nUsedForReferenceFlags |= 2<<(i*2);
			}
			else
			{
				pDXVAPicParams->FieldOrderCntList[i][1]		= 0;
			}

			pDXVAPicParams->RefFrameList[i].AssociatedFlag	= AssociatedFlag;
			pDXVAPicParams->RefFrameList[i].Index7Bits		= (UCHAR)pic->opaque;
    }
		else
		{
			pDXVAPicParams->FrameNumList[i] = 0;
			pDXVAPicParams->FieldOrderCntList[i][0] = 0;
			pDXVAPicParams->FieldOrderCntList[i][1] = 0;
			pDXVAPicParams->RefFrameList[i].AssociatedFlag = 1;
			pDXVAPicParams->RefFrameList[i].Index7Bits = 127;
		}
	}

	pDXVAPicParams->UsedForReferenceFlags	= nUsedForReferenceFlags;
}

BOOL FFH264IsRefFrameInUse (int nFrameNum, struct AVCodecContext* pAVCtx)
{
	H264Context*	h			= (H264Context*) pAVCtx->priv_data;
	int				i;

	for (i=0; i<h->short_ref_count; i++)
	{
		if ((int)h->short_ref[i]->opaque == nFrameNum)
			return TRUE;
	}

	for (i=0; i<h->long_ref_count; i++)
	{
		if ((int)h->long_ref[i]->opaque == nFrameNum)
			return TRUE;
	}

	return FALSE;
}


void FF264UpdateRefFrameSliceLong(DXVA_PicParams_H264* pDXVAPicParams, DXVA_Slice_H264_Long* pSlice, struct AVCodecContext* pAVCtx)
{
	H264Context*			h			= (H264Context*) pAVCtx->priv_data;
    MpegEncContext* const	s = &h->s;
	HRESULT					hr = E_FAIL;
	unsigned int			i;

	for(i=0; i<32; i++)
	{ pSlice->RefPicList[0][i].AssociatedFlag = 1;
	  pSlice->RefPicList[0][i].bPicEntry = 255; 
	  pSlice->RefPicList[0][i].Index7Bits = 127;
	  pSlice->RefPicList[1][i].AssociatedFlag = 1; 
	  pSlice->RefPicList[1][i].bPicEntry = 255;
	  pSlice->RefPicList[1][i].Index7Bits = 127;
	}

	if(h->slice_type != FF_I_TYPE && h->slice_type != FF_SI_TYPE) 
	{
		if(h->ref_count[0] > 0){
			for(i=0; i < h->ref_count[0]; i++){
//				   pSlice->RefPicList[0][i].Index7Bits = h->ref_list[0][i].frame_num; //nSurfaceIndex; TODO : check this !
			   pSlice->RefPicList[0][i].Index7Bits = FFH264FindRefFrameIndex (h->ref_list[0][i].frame_num, pDXVAPicParams);
			   pSlice->RefPicList[0][i].AssociatedFlag = 0;
			   if((h->s.picture_structure != PICT_FRAME)){
			     if((h->sei_pic_struct == SEI_PIC_STRUCT_BOTTOM_FIELD) || 
					   (h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM) ||
					   (h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM_TOP)){
						   pSlice->RefPicList[0][i].AssociatedFlag = 1; 
				   }
				 
			   }

			}
		}
	}
	else
		pSlice->num_ref_idx_l0_active_minus1 = 0;

	if(h->slice_type == FF_B_TYPE || h->slice_type == FF_S_TYPE || h->slice_type == FF_BI_TYPE) 
	{
		if(h->ref_count[1] > 0){
			for(i=0; i < h->ref_count[1]; i++){
			   pSlice->RefPicList[1][i].Index7Bits = FFH264FindRefFrameIndex (h->ref_list[1][i].frame_num, pDXVAPicParams);
			   pSlice->RefPicList[1][i].AssociatedFlag = 0; 
			   if((h->s.picture_structure != PICT_FRAME)){
				   if((h->sei_pic_struct == SEI_PIC_STRUCT_BOTTOM_FIELD) || 
					   (h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM) ||
					   (h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM_TOP)){
						   pSlice->RefPicList[1][i].AssociatedFlag = 1; 
				   }
			   }
			}
		}
	}
	else
		pSlice->num_ref_idx_l1_active_minus1 = 0;
	
	
	if(h->slice_type == FF_I_TYPE || h->slice_type == FF_SI_TYPE) 
	{
		for(i = 0; i<16; i++)
			pSlice->RefPicList[0][i].bPicEntry = 0xff; 
	}

	if(h->slice_type == FF_P_TYPE || h->slice_type == FF_I_TYPE || 
	   h->slice_type ==FF_SP_TYPE  || h->slice_type == FF_SI_TYPE) 
	{	
		for(i = 0; i < 16; i++) 
		 pSlice->RefPicList[1][i].bPicEntry = 0xff; 
	} 
}

void FFH264SetDxvaSliceLong (struct AVCodecContext* pAVCtx, void* pSliceLong)
{
	H264Context*	h = (H264Context*) pAVCtx->priv_data;
	h->dxva_slice_long = pSliceLong;
}

HRESULT FFVC1UpdatePictureParam (DXVA_PictureParameters* pPicParams, struct AVCodecContext* pAVCtx, int* nFieldType, int* nSliceType, BYTE* pBuffer, UINT nSize)
{
	VC1Context*		vc1 = (VC1Context*) pAVCtx->priv_data;

	if (pBuffer)
	{
		av_vc1_decode_frame (pAVCtx, pBuffer, nSize);
	}

	// WARNING : vc1->interlace is not reliable (always set for progressive video on HD-DVD material)
	if (vc1->fcm == 0)
		*nFieldType = PICT_FRAME;
	else	// fcm : 2 or 3 frame or field interlaced
		*nFieldType = (vc1->tff ? PICT_TOP_FIELD : PICT_BOTTOM_FIELD);

	pPicParams->bPicIntra				= (vc1->s.pict_type == FF_I_TYPE);
	pPicParams->bPicBackwardPrediction	= (vc1->s.pict_type == FF_B_TYPE);

	// Init    Init    Init    Todo      
	// iWMV9 - i9IRU - iOHIT - iINSO - iWMVA - 0 - 0 - 0		| Section 3.2.5
	pPicParams->bBidirectionalAveragingMode	= (pPicParams->bBidirectionalAveragingMode & 0xE0) |	// init in SetExtraData
											  ((vc1->lumshift!=0 || vc1->lumscale!=32) ? 0x10 : 0)| // iINSO
											  ((vc1->profile == PROFILE_ADVANCED)	 <<3 );			// iWMVA

	// Section 3.2.20.3
	pPicParams->bPicSpatialResid8	= (vc1->panscanflag   << 7) | (vc1->refdist_flag << 6) |
									  (vc1->s.loop_filter << 5) | (vc1->fastuvmc     << 4) | 
									  (vc1->extended_mv   << 3) | (vc1->dquant       << 1) | 
									  (vc1->vstransform);

	// Section 3.2.20.4
	pPicParams->bPicOverflowBlocks  = (vc1->quantizer_mode  << 6) | (vc1->multires << 5) |
									  (vc1->s.resync_marker << 4) | (vc1->rangered << 3) |
									  (vc1->s.max_b_frames);

	// Section 3.2.20.2
	pPicParams->bPicDeblockConfined	= (vc1->postprocflag << 7) | (vc1->broadcast  << 6) |
									  (vc1->interlace    << 5) | (vc1->tfcntrflag << 4) | 
									  (vc1->finterpflag  << 3) | // (refpic << 2) set in DecodeFrame !
									  (vc1->psf << 1)		   | vc1->extended_dmv;


	//				TODO section 3.2.20.6
	pPicParams->bPicStructure		= *nFieldType;

	// Cf page 17 : 2 for interlaced, 0 for progressive
	pPicParams->bPicExtrapolation = (*nFieldType == PICT_FRAME) ? 1 : 2;

	pPicParams->wBitstreamPCEelements	= vc1->lumshift;
	pPicParams->wBitstreamFcodes		= vc1->lumscale;

	// Section 3.2.16
	*nSliceType = vc1->s.pict_type;

	// TODO : not finish...
	pPicParams->bMVprecisionAndChromaRelation = ((vc1->mv_mode == MV_PMODE_1MV_HPEL_BILIN) << 3) |		// 0 for non-bilinear luma motion, 1 for bilinear
												(1 << 2) |		// 0 for WMV8, 1 for WMV9 motion
												(0 << 1) |		// 1 for WMV8 quarter sample luma motion
												(0);			// 0 for quarter sample chroma motion, 1 for half sample chroma

	// Cf �7.1.1.25 in VC1 specification, �3.2.14.3 in DXVA spec
	pPicParams->bRcontrol	= vc1->rnd;

	/*
	// TODO : find files with de-ringing  ...
	pPicParams->bPicDeblocked	= ((vc1->postproc & 0x01) ? 0x02 : 0) |	// In loop de-blocking
								  ((vc1->postproc & 0x02) ? 0x08 : 0);	// Out of loop de-ringing
	*/

	return S_OK;
}

unsigned long FFGetMBNumber(struct AVCodecContext* pAVCtx)
{
	VC1Context*		vc1 = NULL;
	H264Context*	h	= NULL;

	switch (pAVCtx->codec_id)
	{
	case CODEC_ID_VC1 :
		vc1 = (VC1Context*) pAVCtx->priv_data;
		return vc1->s.mb_num;
	case CODEC_ID_H264 :
		h	= (H264Context*) pAVCtx->priv_data;
		return h->s.mb_num;
	}

	return 0;
}

int FFIsSkipped(struct AVCodecContext* pAVCtx)
{
	VC1Context*		vc1 = (VC1Context*) pAVCtx->priv_data;
	return vc1->p_frame_skipped;
}

int FFIsInterlaced(struct AVCodecContext* pAVCtx, int nHeight)
{
	if (pAVCtx->codec_id == CODEC_ID_H264)
	{
		H264Context*	h		= (H264Context*) pAVCtx->priv_data;
		SPS*			cur_sps = h->sps_buffers[0];

		if (cur_sps && !cur_sps->frame_mbs_only_flag)
			return 1;
		else
			return 0;
	}
	else if (pAVCtx->codec_id == CODEC_ID_VC1)
	{
		VC1Context*		vc1 = (VC1Context*) pAVCtx->priv_data;
		return vc1->interlace;
	}

	return 0;
}

void FFSetThreadNumber(struct AVCodecContext* pAVCtx, int nThreadCount)
{
	if (pAVCtx->thread_count > 1)
	{
		avcodec_thread_free (pAVCtx);
		pAVCtx->thread_count = 1;
	}

	if (nThreadCount > 1)
		avcodec_thread_init(pAVCtx, nThreadCount);
}

#endif // HAS_DX