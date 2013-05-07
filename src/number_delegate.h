#ifndef NUMBER_DELEGATE
#define NUMBER_DELEGATE

#include <QString>


class NumberDelegate
{
	public:
		NumberDelegate();
        virtual ~NumberDelegate() {}
		
		int integerDigits() { return d_intDigits; }
		int decimalDigits() { return d_decDigits; }
		
		/** \brief decodes on the basis of the format, correcting integer and decimal digits with minimum 
		 *         and maximum values
		 */
		void configureNumber(const double min, const double max, const QString& format);
		
		/** \brief decodes on the basis of the format only
		 */
		bool configureNumber(const QString& format);
		
	protected:
		
		int d_intDigits, d_decDigits;
};

#endif
