#ifndef CFORMATTED_INJECT_HPP_
#define CFORMATTED_INJECT_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/phoenix/stl/container.hpp>
#include <boost/phoenix/bind/bind_function.hpp>

#include "CPacket.hpp"

namespace FormattedInject
{
	VOID DoEncode1(CPacket* pckt, BOOL bHidden, BYTE b);
	VOID DoEncode2(CPacket* pckt, BOOL bHidden, WORD w);
	VOID DoEncode4(CPacket* pckt, BOOL bHidden, DWORD dw);
	VOID DoEncode8(CPacket* pckt, BOOL bHidden, ULONGLONG ull);
	VOID DoEncodeString(CPacket* pckt, BOOL bHidden, std::string& str);
	VOID DoEncodeBuffer(CPacket* pckt, BOOL bHidden, std::vector<BYTE>& vb);
}

template <typename Iterator>
struct formatted_packet_grammar : boost::spirit::qi::grammar<Iterator, boost::spirit::ascii::space_type>
{
	formatted_packet_grammar(CPacket* oPacket, BOOL bHide) : pPacket(oPacket), bHidden(bHide), formatted_packet_grammar::base_type(start)
	{
		QUOTED_STRING_ %= boost::spirit::qi::lexeme['"' >> *(boost::spirit::ascii::char_ - '"') >> '"'];
		ARRAY_ %= '[' >> boost::spirit::qi::lexeme[+BYTE_] >> ']';

		start =
			boost::spirit::qi::repeat
			[
				QUOTED_STRING_[boost::phoenix::bind(&FormattedInject::DoEncodeString, pPacket, bHidden, boost::spirit::_1)]
				|
				ULONGLONG_[boost::phoenix::bind(&FormattedInject::DoEncode8, pPacket, bHidden, boost::spirit::_1)]
				|
				DWORD_[boost::phoenix::bind(&FormattedInject::DoEncode4, pPacket, bHidden, boost::spirit::_1)]
				|
				WORD_[boost::phoenix::bind(&FormattedInject::DoEncode2, pPacket, bHidden, boost::spirit::_1)]
				|
				BYTE_[boost::phoenix::bind(&FormattedInject::DoEncode1, pPacket, bHidden, boost::spirit::_1)]
				|
				ARRAY_[boost::phoenix::bind(&FormattedInject::DoEncodeBuffer, pPacket, bHidden, boost::spirit::_1)]
			];
	};

	boost::spirit::qi::rule<Iterator, boost::spirit::ascii::space_type> start;
	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type> QUOTED_STRING_;
	boost::spirit::qi::rule<Iterator, std::vector<BYTE>(), boost::spirit::ascii::space_type> ARRAY_;
	boost::spirit::qi::uint_parser<ULONGLONG, 16, 16, 16> ULONGLONG_;
	boost::spirit::qi::uint_parser<DWORD, 16, 8, 8> DWORD_;
	boost::spirit::qi::uint_parser<WORD, 16, 4, 4> WORD_;
	boost::spirit::qi::uint_parser<BYTE, 16, 2, 2> BYTE_;

	CPacket*	pPacket;
	BOOL        bHidden;
};

typedef std::string::const_iterator iterator_type;
typedef formatted_packet_grammar<iterator_type> formatted_packet_parser;

#endif // CFORMATTED_INJECT_HPP_