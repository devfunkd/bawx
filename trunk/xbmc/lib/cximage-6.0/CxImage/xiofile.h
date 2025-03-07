#if !defined(__xiofile_h)
#define __xiofile_h

#include "xfile.h"
#include <sys/stat.h>

#if defined(_WIN32) && defined (XBMC)
  // we must undefine the macro version of feof - we want to be able to wrap this in our dll loader
  #undef feof
  #define fileno _fileno
#endif

class DLL_EXP CxIOFile : public CxFile
	{
public:
	CxIOFile(FILE* fp = NULL)
	{
		m_fp = fp;
		m_bCloseFile = (bool)(fp==0);
    m_bEof = false;
	}

	~CxIOFile()
	{
		Close();
	}
//////////////////////////////////////////////////////////
	bool Open(const char* filename, const char* mode)
	{
		if (m_fp) return false;	// Can't re-open without closing first

		m_fp = fopen(filename, mode);
		if (!m_fp) return false;

		m_bCloseFile = true;

		return true;
	}
//////////////////////////////////////////////////////////
	virtual bool Close()
	{
		int iErr = 0;
    m_bEof = false;
		if ( (m_fp) && (m_bCloseFile) ){ 
			iErr = fclose(m_fp);
			m_fp = NULL;
		}
		return (bool)(iErr==0);
	}
//////////////////////////////////////////////////////////
	virtual size_t	Read(void *buffer, size_t size, size_t count)
	{
		if (!m_fp) return 0;
		size_t items = fread(buffer, size, count, m_fp);
    if (items < count)
      m_bEof = true;
    return items;
	}
//////////////////////////////////////////////////////////
	virtual size_t	Write(const void *buffer, size_t size, size_t count)
	{
		if (!m_fp) return 0;
		size_t items = fwrite(buffer, size, count, m_fp);
    if (items < count)
      m_bEof = true;
    return items;
	}
//////////////////////////////////////////////////////////
	virtual bool Seek(long offset, int origin)
	{
		if (!m_fp) return false;
		return (bool)(fseek(m_fp, offset, origin) == 0);
	}
//////////////////////////////////////////////////////////
	virtual long Tell()
	{
		if (!m_fp) return 0;
		return ftell(m_fp);
	}
//////////////////////////////////////////////////////////
	virtual long	Size()
	{
		if (!m_fp) return -1;
    struct stat st;
    if (fstat(fileno(m_fp), &st) == -1)
      return -1;
    return st.st_size;
	}
//////////////////////////////////////////////////////////
	virtual bool	Flush()
	{
		if (!m_fp) return false;
		return (bool)(fflush(m_fp) == 0);
	}
//////////////////////////////////////////////////////////
	virtual bool	Eof()
	{
		if (!m_fp || m_bEof) return true;
		return (bool)(feof(m_fp) != 0);
	}
//////////////////////////////////////////////////////////
	virtual long	Error()
	{
		if (!m_fp) return -1;
		return ferror(m_fp);
	}
//////////////////////////////////////////////////////////
	virtual bool PutC(unsigned char c)
	{
		if (!m_fp) return false;
		bool bOk = (bool)(fputc(c, m_fp) == c);
    if (!bOk)
      m_bEof = true;
    return bOk;
	}
//////////////////////////////////////////////////////////
	virtual long	GetC()
	{
		if (!m_fp) return EOF;
		long n = getc(m_fp);
    if (n == EOF)
      m_bEof = true;
    return n;
	}
//////////////////////////////////////////////////////////
	virtual char *	GetS(char *string, int n)
	{
		if (!m_fp) return NULL;
		return fgets(string,n,m_fp);
	}
//////////////////////////////////////////////////////////
	virtual long	Scanf(const char *format, void* output)
	{
		if (!m_fp) return EOF;
		return fscanf(m_fp, format, output);
	}
//////////////////////////////////////////////////////////
protected:
	FILE *m_fp;
	bool m_bCloseFile;
  bool m_bEof;
	};

#endif
