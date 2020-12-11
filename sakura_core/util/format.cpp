﻿/*! @file */
/*
	Copyright (C) 2007, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "format.h"

/*!	日時をフォーマット

	@param[out] szResult 書式変換後の文字列
	@param[in] size szResultに格納可能な文字数(ヌル文字を含む)
	@param[in] format 書式
	@param[in] systime 書式化したい日時
	@retval true  書式文字列の変換が完了
	@retval false 書式文字列の変換が一部または全部失敗

	@note  %Y %y %m %d %H %M %S の変換に対応

	@author aroka
	@date 2005.11.21 新規
*/
bool GetDateTimeFormat( WCHAR* szResult, size_t size, const WCHAR* format, const SYSTEMTIME& systime )
{
	WCHAR szTime[10] = {};
	const WCHAR *p = format;
	WCHAR *q = szResult;
	bool ret = false;

	if( szResult == NULL || size == 0 || format == NULL ){
		return false;
	}

	while( *p ){
		const size_t remains = (size - 1) - (q - szResult);
		if( remains <= 0 ){
			break;
		}

		int timeLen = -1;
		if( *p == L'%' ){
			++p;
			switch( *p ){
			case L'Y':
				timeLen = wsprintf( szTime, L"%d", systime.wYear );
				break;
			case L'y':
				timeLen = wsprintf( szTime, L"%02d", (systime.wYear % 100) );
				break;
			case L'm':
				timeLen = wsprintf( szTime, L"%02d", systime.wMonth );
				break;
			case L'd':
				timeLen = wsprintf( szTime, L"%02d", systime.wDay );
				break;
			case L'H':
				timeLen = wsprintf( szTime, L"%02d", systime.wHour );
				break;
			case L'M':
				timeLen = wsprintf( szTime, L"%02d", systime.wMinute );
				break;
			case L'S':
				timeLen = wsprintf( szTime, L"%02d", systime.wSecond );
				break;
				// A Z
			case L'%':
			default:
				break;
			}
		}

		if( 0 <= timeLen ){
			if( remains < timeLen ){
				break;
			}
			wcscpy( q, szTime );
			++p;
			q += timeLen;
		}else{
			if( *p != L'\0' ){
				*q++ = *p++;
			}
		}
	}

	*q = L'\0';

	if( *p == L'\0' ){
		ret = true;
	}

	return ret;
}

/*!	バージョン番号の解析

	@param[in] バージョン番号文字列
	@return UINT32 8bit（符号1bit+数値7bit）ずつメジャー、マイナー、ビルド、リビジョンを格納

	@author syat
	@date 2011.03.18 新規
	@note 参考 PHP version_compare http://php.s3.to/man/function.version-compare.html
*/
UINT32 ParseVersion( const WCHAR* sVer )
{
	int nVer;
	int nShift = 0;	//特別な文字列による下駄
	int nDigit = 0;	//連続する数字の数
	UINT32 ret = 0;

	const WCHAR *p = sVer;
	int i;

	for( i=0; *p && i<4; i++){
		//特別な文字列の処理
		if( *p == L'a' ){
			if( wcsncmp_literal( p, L"alpha" ) == 0 )p += 5;
			else p++;
			nShift = -0x60;
		}
		else if( *p == L'b' ){
			if( wcsncmp_literal( p, L"beta" ) == 0 )p += 4;
			else p++;
			nShift = -0x40;
		}
		else if( *p == L'r' || *p == L'R' ){
			if( wcsnicmp_literal( p, L"rc" ) == 0 )p += 2;
			else p++;
			nShift = -0x20;
		}
		else if( *p == L'p' ){
			if( wcsncmp_literal( p, L"pl" ) == 0 )p += 2;
			else p++;
			nShift = 0x20;
		}
		else if( !_istdigit(*p) ){
			nShift = -0x80;
		}
		else{
			nShift = 0;
		}
		while( *p && !_istdigit(*p) ){ p++; }
		//数値の抽出
		for( nVer = 0, nDigit = 0; _istdigit(*p); p++ ){
			if( ++nDigit > 2 )break;	//数字は2桁までで止める
			nVer = nVer * 10 + *p - L'0';
		}
		//区切り文字の処理
		while( *p && wcschr( L".-_+", *p ) ){ p++; }

		DEBUG_TRACE(L"  VersionPart%d: ver=%d,shift=%d\n", i, nVer, nShift);
		ret |= ( (nShift + nVer + 128) << (24-8*i) );
	}
	for( ; i<4; i++ ){	//残りの部分はsigned 0 (=0x80)を埋める
		ret |= ( 128 << (24-8*i) );
	}

	DEBUG_TRACE(L"ParseVersion %ls -> %08x\n", sVer, ret);
	return ret;
}

/*!	バージョン番号の比較

	@param[in] バージョンA
	@param[in] バージョンB
	@return int 0: バージョンは等しい、1以上: Aが新しい、-1以下: Bが新しい

	@author syat
	@date 2011.03.18 新規
*/
int CompareVersion( const WCHAR* verA, const WCHAR* verB )
{
	UINT32 nVerA = ParseVersion(verA);
	UINT32 nVerB = ParseVersion(verB);

	return nVerA - nVerB;
}
