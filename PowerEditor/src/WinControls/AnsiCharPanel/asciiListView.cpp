// This file is part of Notepad++ project
// Copyright (C)2021 Don HO <don.h@free.fr>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "asciiListView.h"
#include "Parameters.h"

using namespace std;

void AsciiListView::resetValues(int codepage)
{
	if (codepage == -1)
		codepage = 0;

	if (_codepage == codepage)
		return;

	ListView_DeleteAllItems(_hSelf);
	setValues(codepage);
}

wstring AsciiListView::getAscii(unsigned char value)
{
	switch (value)
	{
		case 0:
			return "NULL";
		case 1:
			return "SOH";
		case 2:
			return "STX";
		case 3:
			return "ETX";
		case 4:
			return "EOT";
		case 5:
			return "ENQ";
		case 6:
			return "ACK";
		case 7:
			return "BEL";
		case 8:
			return "BS";
		case 9:
			return "TAB";
		case 10:
			return "LF";
		case 11:
			return "VT";
		case 12:
			return "FF";
		case 13:
			return "CR";
		case 14:
			return "SO";
		case 15:
			return "SI";
		case 16:
			return "DLE";
		case 17:
			return "DC1";
		case 18:
			return "DC2";
		case 19:
			return "DC3";
		case 20:
			return "DC4";
		case 21:
			return "NAK";
		case 22:
			return "SYN";
		case 23:
			return "ETB";
		case 24:
			return "CAN";
		case 25:
			return "EM";
		case 26:
			return "SUB";
		case 27:
			return "ESC";
		case 28:
			return "FS";
		case 29:
			return "GS";
		case 30:
			return "RS";
		case 31:
			return "US";
		case 32:
			return "Space";
		case 127:
			return "DEL";
		default:
		{
			NppChar charStr[10]{};
			char ascii[2]{};
			ascii[0] = value;
			ascii[1] = '\0';
			nppMBtoWC(_codepage, 0, ascii, -1, charStr, _countof(charStr));
			return charStr;
		}

	}
}

wstring AsciiListView::getHtmlName(unsigned char value)
{
	switch (value)
	{
		case 33:
			return "&excl;";
		case 34:
			return "&quot;";
		case 35:
			return "&num;";
		case 36:
			return "&dollar;";
		case 37:
			return "&percnt;";
		case 38:
			return "&amp;";
		case 39:
			return "&apos;";
		case 40:
			return "&lpar;";
		case 41:
			return "&rpar;";
		case 42:
			return "&ast;";
		case 43:
			return "&plus;";
		case 44:
			return "&comma;";
		case 45:
			return "&minus;";
		case 46:
			return "&period;";
		case 47:
			return "&sol;";
		case 58:
			return "&colon;";
		case 59:
			return "&semi;";
		case 60:
			return "&lt;";
		case 61:
			return "&equals;";
		case 62:
			return "&gt;";
		case 63:
			return "&quest;";
		case 64:
			return "&commat;";
		case 91:
			return "&lbrack;";
		case 92:
			return "&bsol;";
		case 93:
			return "&rbrack;";
		case 94:
			return "&Hat;";
		case 95:
			return "&lowbar;";
		case 96:
			return "&grave;";
		case 123:
			return "&lbrace;";
		case 124:
			return "&vert;";
		case 125:
			return "&rbrace;";
		case 126:
			return ""; // ascii tilde
		case 128:
			return "&euro;";
		case 130:
			return "&sbquo;";
		case 131:
			return "&fnof;";
		case 132:
			return "&bdquo;";
		case 133:
			return "&hellip;";
		case 134:
			return "&dagger;";
		case 135:
			return "&Dagger;";
		case 136:
			return "&circ;";
		case 137:
			return "&permil;";
		case 138:
			return "&Scaron;";
		case 139:
			return "&lsaquo;";
		case 140:
			return "&OElig;";
		case 142:
			return "&Zcaron;";
		case 145:
			return "&lsquo;";
		case 146:
			return "&rsquo;";
		case 147:
			return "&ldquo;";
		case 148:
			return "&rdquo;";
		case 149:
			return "&bull;";
		case 150:
			return "&ndash;";
		case 151:
			return "&mdash;";
		case 152:
			return "&tilde;";
		case 153:
			return "&trade;";
		case 154:
			return "&scaron;";
		case 155:
			return "&rsaquo;";
		case 156:
			return "&oelig;";
		case 158:
			return "&zcaron;";
		case 159:
			return "&Yuml;";
		case 160:
			return "&nbsp;";
		case 161:
			return "&iexcl;";
		case 162:
			return "&cent;";
		case 163:
			return "&pound;";
		case 164:
			return "&curren;";
		case 165:
			return "&yen;";
		case 166:
			return "&brvbar;";
		case 167:
			return "&sect;";
		case 168:
			return "&uml;";
		case 169:
			return "&copy;";
		case 170:
			return "&ordf;";
		case 171:
			return "&laquo;";
		case 172:
			return "&not;";
		case 173:
			return "&shy;";
		case 174:
			return "&reg;";
		case 175:
			return "&macr;";
		case 176:
			return "&deg;";
		case 177:
			return "&plusmn;";
		case 178:
			return "&sup2;";
		case 179:
			return "&sup3;";
		case 180:
			return "&acute;";
		case 181:
			return "&micro;";
		case 182:
			return "&para;";
		case 183:
			return "&middot;";
		case 184:
			return "&cedil;";
		case 185:
			return "&sup1;";
		case 186:
			return "&ordm;";
		case 187:
			return "&raquo;";
		case 188:
			return "&frac14;";
		case 189:
			return "&frac12;";
		case 190:
			return "&frac34;";
		case 191:
			return "&iquest;";
		case 192:
			return "&Agrave;";
		case 193:
			return "&Aacute;";
		case 194:
			return "&Acirc;";
		case 195:
			return "&Atilde;";
		case 196:
			return "&Auml;";
		case 197:
			return "&Aring;";
		case 198:
			return "&AElig;";
		case 199:
			return "&Ccedil;";
		case 200:
			return "&Egrave;";
		case 201:
			return "&Eacute;";
		case 202:
			return "&Ecirc;";
		case 203:
			return "&Euml;";
		case 204:
			return "&Igrave;";
		case 205:
			return "&Iacute;";
		case 206:
			return "&Icirc;";
		case 207:
			return "&Iuml;";
		case 208:
			return "&ETH;";
		case 209:
			return "&Ntilde;";
		case 210:
			return "&Ograve;";
		case 211:
			return "&Oacute;";
		case 212:
			return "&Ocirc;";
		case 213:
			return "&Otilde;";
		case 214:
			return "&Ouml;";
		case 215:
			return "&times;";
		case 216:
			return "&Oslash;";
		case 217:
			return "&Ugrave;";
		case 218:
			return "&Uacute;";
		case 219:
			return "&Ucirc;";
		case 220:
			return "&Uuml;";
		case 221:
			return "&Yacute;";
		case 222:
			return "&THORN;";
		case 223:
			return "&szlig;";
		case 224:
			return "&agrave;";
		case 225:
			return "&aacute;";
		case 226:
			return "&acirc;";
		case 227:
			return "&atilde;";
		case 228:
			return "&auml;";
		case 229:
			return "&aring;";
		case 230:
			return "&aelig;";
		case 231:
			return "&ccedil;";
		case 232:
			return "&egrave;";
		case 233:
			return "&eacute;";
		case 234:
			return "&ecirc;";
		case 235:
			return "&euml;";
		case 236:
			return "&igrave;";
		case 237:
			return "&iacute;";
		case 238:
			return "&icirc;";
		case 239:
			return "&iuml;";
		case 240:
			return "&eth;";
		case 241:
			return "&ntilde;";
		case 242:
			return "&ograve;";
		case 243:
			return "&oacute;";
		case 244:
			return "&ocirc;";
		case 245:
			return "&otilde;";
		case 246:
			return "&ouml;";
		case 247:
			return "&divide;";
		case 248:
			return "&oslash;";
		case 249:
			return "&ugrave;";
		case 250:
			return "&uacute;";
		case 251:
			return "&ucirc;";
		case 252:
			return "&uuml;";
		case 253:
			return "&yacute;";
		case 254:
			return "&thorn;";
		case 255:
			return "&yuml;";
		default:
		{
			return "";
		}
	}
}

int AsciiListView::getHtmlNumber(unsigned char value)
{
	switch (value)
	{
		case 45:
			return 8722;
		case 128:
			return 8364;
		case 130:
			return 8218;
		case 131:
			return 402;
		case 132:
			return 8222;
		case 133:
			return 8230;
		case 134:
			return 8224;
		case 135:
			return 8225;
		case 136:
			return 710;
		case 137:
			return 8240;
		case 138:
			return 352;
		case 139:
			return 8249;
		case 140:
			return 338;
		case 142:
			return 381;
		case 145:
			return 8216;
		case 146:
			return 8217;
		case 147:
			return 8220;
		case 148:
			return 8221;
		case 149:
			return 8226;
		case 150:
			return 8211;
		case 151:
			return 8212;
		case 152:
			return 732;
		case 153:
			return 8482;
		case 154:
			return 353;
		case 155:
			return 8250;
		case 156:
			return 339;
		case 158:
			return 382;
		case 159:
			return 376;
		default:
		{
			return -1;
		}
	}
}

void AsciiListView::setValues(int codepage)
{
	_codepage = codepage;

	for (int i = 0 ; i < 256 ; ++i)
	{
		constexpr size_t bufSize = 8;
		constexpr size_t bufSizeHex = 9;
		NppChar dec[bufSize]{};
		NppChar hex[bufSize]{};
		NppChar htmlNumber[bufSize]{};
		NppChar htmlHexNumber[bufSizeHex]{};
		wstring htmlName;
		swprintf(dec, bufSize, "%d", i);
		swprintf(hex, bufSize, "%02X", i);
		wstring s = getAscii(static_cast<unsigned char>(i));

		if (codepage == 0 || codepage == 1252)
		{
			if ((i >= 32 && i <= 126 && i != 45) || (i >= 160 && i <= 255))
			{
				swprintf(htmlNumber, bufSize, "&#%d;", i);
				swprintf(htmlHexNumber, bufSize, "&#x%x;", i);
			}
			else
			{
				int n = getHtmlNumber(static_cast<unsigned char>(i));
				if (n > -1)
				{
					swprintf(htmlNumber, bufSize, "&#%d;", n);
					swprintf(htmlHexNumber, bufSizeHex, "&#x%x;", n);
				}
				else
				{
					swprintf(htmlNumber, bufSize, "");
					swprintf(htmlHexNumber, bufSizeHex, "");
				}
			}

			htmlName = getHtmlName(static_cast<unsigned char>(i));
		}
		else
		{
			swprintf(htmlNumber, bufSize, "");
			swprintf(htmlHexNumber, bufSizeHex, "");
			htmlName = "";
		}

		std::vector<wstring> values2Add;

		values2Add.push_back(dec);
		values2Add.push_back(hex);
		values2Add.push_back(s);
		values2Add.push_back(htmlName);
		values2Add.push_back(htmlNumber);
		values2Add.push_back(htmlHexNumber);

		addLine(values2Add);
	}
}
