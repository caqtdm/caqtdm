#ifndef __VARIANTTOOLS_H__
#define __VARIANTTOOLS_H__

#include <ArrayTools.h>

// from include/ks.h,
// which seems to be the best source about the VARIANT data type
static const char *variant_type_strings[] =
{
    "VT_EMPTY",
    "VT_NULL",
    "VT_I2",
    "VT_I4",
    "VT_R4",
    "VT_R8",
    "VT_CY",
    "VT_DATE",
    "VT_BSTR",
    "VT_DISPATCH",
    "VT_ERROR",
    "VT_BOOL",
    "VT_VARIANT",
    "VT_UNKNOWN",
    "VT_DECIMAL",
	"VT_15??",
    "VT_I1",
    "VT_UI1",
    "VT_UI2",
    "VT_UI4",
    "VT_I8",
    "VT_UI8",
    "VT_INT",
    "VT_UINT",
    "VT_VOID",
    "VT_HRESULT",
    "VT_PTR",
    "VT_SAFEARRAY",
    "VT_CARRAY",
    "VT_USERDEFINED",
    "VT_LPSTR",
    "VT_LPWSTR"
};

// convert variant type into string,
// only for debugging, not thread-safe
inline const char *VariantTypeString (VARIANT &variant)
{
		static char buffer[200];
		int	type = variant.vt & VT_TYPEMASK;

		buffer[0] = '\0';
		if (variant.vt & VT_VECTOR)
			strcat (buffer, "VT_VECTOR | ");
		if (variant.vt & VT_ARRAY)
			strcat (buffer, "VT_ARRAY | ");

		if (type < SIZEOF_ARRAY(variant_type_strings))
			strcat (buffer, variant_type_strings[type]);
		else
			strcat (buffer, "<unknown type>");
		if (variant.vt & VT_BYREF)
			strcat (buffer, " | VT_BYREF");

		return buffer;
}

inline void ShowVariant (VARIANT &v)
{
	VARTYPE basictype = v.vt & VT_TYPEMASK;
	unsigned long i;

	ATLTRACE (_T("VARIANT type %s\n"), VariantTypeString(v));

	// handle arrays
	if (v.vt & VT_ARRAY)
	{
		UINT dim = SafeArrayGetDim(v.parray);
		long lower, upper;
		unsigned long size;
		void HUGEP *data;

		//	'1' for first dimension, not '0' !
		for (i=1; i<=dim; ++i)
		{
			SafeArrayGetLBound(v.parray, i, &lower);
			SafeArrayGetUBound(v.parray, i, &upper);
			ATLTRACE(_T("bounds for dim %d: %ld ... %ld\n"),
				i, lower, upper);
		}
		SafeArrayGetLBound(v.parray, 1, &lower);
		SafeArrayGetUBound(v.parray, 1, &upper);
		size = upper - lower + 1; // lower..upper -> 0...size-1

		SafeArrayAccessData (v.parray, &data);
		switch (basictype)
		{
		case VT_I2:
			for (i=0; i<size; ++i)
				ATLTRACE(_T("[%d] = %d\n"), i, (int) ((short *)data)[i]);
			break;
		case VT_I4:
			for (i=0; i<size; ++i)
				ATLTRACE(_T("[%d] = %ld\n"), i, ((long *)data)[i]);
			break;
		case VT_R4:
			for (i=0; i<size; ++i)
				ATLTRACE(_T("[%d] = %g\n"), i, (double) ((float *)data)[i]);
			break;
		case VT_R8:
			for (i=0; i<size; ++i)
				ATLTRACE(_T("[%d] = %g\n"), i, ((double *)data)[i]);
			break;
		default:
			ATLTRACE(_T("(cannot list contents for this type)\n"));
		}
		SafeArrayUnaccessData (v.parray);

		return;
	}

    size_t len;
    char *c;
	// no array
	switch (basictype)
	{
	case VT_I2:
		ATLTRACE(_T("value: %d\n"), (int) v.iVal);
		break;
	case VT_I4:
		ATLTRACE(_T("value: %ld\n"), (long) v.lVal);
		break;
	case VT_R4:
		ATLTRACE(_T("value: %g\n"), (double) v.fltVal);
		break;
	case VT_R8:
		ATLTRACE(_T("value: %g\n"), (double) v.dblVal);
		break;
	case VT_BSTR:
		ATLTRACE(_T("value: BSTR "));
        len = SysStringLen(v.bstrVal);
        c = (char *) v.bstrVal;
        for (i=0; i<len; ++i)
        {
		    ATLTRACE(_T("%c"), *c);
            c += 2;
        }
        break;
break;
    default:
		ATLTRACE(_T("(cannot list contents for this type)\n"));
	}
}

#endif //__VARIANTTOOLS_H__
