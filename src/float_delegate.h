#ifndef FLOAT_DELEGATE_H
#define FLOAT_DELEGATE_H

#include "number_delegate.h"

class FloatDelegate : public NumberDelegate
{
	public:
		FloatDelegate() : NumberDelegate() {};
		
		virtual ~FloatDelegate() {};
	
		virtual void setValue(double) = 0;
		virtual void setMinimum(double) = 0;
		virtual void setMaximum(double) = 0;
		virtual void setIntDigits(int) = 0;
		virtual void setDecDigits(int) = 0;
};


#endif
