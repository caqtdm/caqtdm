#ifndef DEC_INT_CONVERTER_H
#define DEC_INT_CONVERTER_H

#include <QString>

/** \brief Given a format expressed by a string, this class decodes it and extracts the number of integer and decimal digits
 *
 * Given a format expressed as QString in the constructor, the decode() method decodes the format and tells 
 * how many integer and decimal digits are needed.
 * In this first simple implementation, numbers of the form ddd.ddd are decoded, exponentials are not.
 */
class DecIntFromFormat
{
	public:
		/** \brief the constructor receiving the format to be converted.
		 *
		 * @param f the format.
		 * <strong>Examples</strong>
		 * <ul>
		 * <li><em>%d</em>, an integer, default values for the number of digits: 3 integers 0 decimals;</li>
		 * <li><em>%f</em>, a float, default values for the number of digits: 3 integers, 2 decimals;</li>
		 * <li><em>%.2f</em> or <em>%'.2f</em>, a float with 2 decimals, default number of digits</li>
		 * <li><em>%3.2f</em>, a float with three  integers and 2 decimals</li>
		 * <li><em>%4d</em> or <em>%.4d</em>, integer with four digits;</li>
		 * </ul>
		 * <strong>Note</strong> <p>Formats containing <em>`</em> are understood but haven't any
		 * effect on the output number of integers and decimals.</p>
		 */
		DecIntFromFormat(QString f);
		
		int numIntDigits() { return d_intDigits;}
		int numDecDigits() { return d_decDigits; }
		
		bool error() { return d_error; }
		QString errorMsg() { return d_errorMsg; }
		
		bool integerDefaults() { return d_intDefaults; }
		bool decimalDefaults() { return d_decDefaults; }
		
		/** \brief decoding method
		 *
		 * After construction, call decode on your object to retrieve later numIntDigits() and numDecDigits()
		 * @return true if the decodification was successful, false otherwise.
		 */
		bool decode();
		
		private:
			bool d_error, d_intDefaults, d_decDefaults;
			int d_intDigits, d_decDigits;
			QString d_errorMsg;
			QString d_format;
};

#endif

