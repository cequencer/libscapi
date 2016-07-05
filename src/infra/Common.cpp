/**
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
* Copyright (c) 2016 LIBSCAPI (http://crypto.biu.ac.il/SCAPI)
* This file is part of the SCAPI project.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
* We request that any publication and/or code referring to and/or based on SCAPI contain an appropriate citation to SCAPI, including a reference to
* http://crypto.biu.ac.il/SCAPI.
* 
* Libscapi uses several open source libraries. Please see these projects for any further licensing issues.
* For more information , See https://github.com/cryptobiu/libscapi/blob/master/LICENSE.MD
*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* 
*/


#include <boost/multiprecision/miller_rabin.hpp>
#include "../../include/infra/Common.hpp"
#include <chrono>
#include <boost/multiprecision/cpp_dec_float.hpp>

/******************************/
/* Helper Methods *************/
/******************************/

int find_log2_floor(biginteger bi) {
	if (bi < 0)
		throw runtime_error("log for negative number is not supported");
	int r = 0;
	while (bi >>= 1) // unroll for more speed...
		r++;
	return r;
}

int NumberOfBits(const biginteger bi) {
	auto bis = (bi>0) ? bi : -bi;
	return find_log2_floor(bis)+ 1;
}

void gen_random_bytes_vector(vector<byte> &v, const int len, mt19937 & random) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) 
		v.push_back(alphanum[random() % (sizeof(alphanum) - 1)]);
}

/**
* Copies all byte from source vector to dest starting from some index in dest.
* Assuming dest is already initialized.
*/
void copy_byte_vector_to_byte_array(const vector<byte> &source_vector, byte * dest, int beginIndex) {
	for (auto it = source_vector.begin(); it != source_vector.end(); ++it) {
		int index = std::distance(source_vector.begin(), it) + beginIndex;
		dest[index] = *it;
	}
}

void copy_byte_array_to_byte_vector(const byte* src, int src_len, vector<byte>& target_vector, int beginIndex)
{
	target_vector.insert(target_vector.end(), &src[beginIndex], &src[src_len]);
}

/*
* Length of biginteger in bytes
*/
size_t bytesCount(biginteger raw_value)
{
#ifdef _WIN32
	auto value = raw_value;
#else
	auto value = raw_value.convert_to<mp::cpp_int>();
#endif
	if (value.is_zero())
		return 1;
	if (value.sign() < 0)
		value = ~value;
	size_t length = 0;
	byte lastByte;
	do {
		lastByte = value.convert_to<byte>();
		value >>= 8;
		length++;
	} while (!value.is_zero());
	if (lastByte >= 0x80)
		length++;
	return length;
}

void encodeBigInteger(biginteger raw_value, byte* output, size_t length)
{
#ifdef _WIN32
	auto value = raw_value;
#else
	auto value = raw_value.convert_to<mp::cpp_int>();
#endif

	if (value.is_zero())
		*output = 0;
	else if (value.sign() > 0)
		while (length-- > 0) {
			*(output++) = value.convert_to<byte>();
			value >>= 8;
		}
	else {
		value = ~value;
		while (length-- > 0) {
			*(output++) = ~value.convert_to<byte>();
			value >>= 8;
		}
	}
}

const vector<string> explode(const string& s, const char& c)
{
	string buff{ "" };
	vector<string> v;

	for (auto n : s)
	{
		if (n != c) buff += n; else
			if (n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if (buff != "") v.push_back(buff);

	return v;
}


biginteger decodeBigInteger(byte* input, size_t length)
{
	biginteger result(0);
	int bits = -8;
	while (length-- > 1)
		result |= (biginteger) *(input++) << (bits += 8);
	byte a = *(input++);
	result |= (biginteger) a << (bits += 8);
	if (a >= 0x80)
		result |= (biginteger) - 1 << (bits + 8);
	return result;
	
#ifdef _WIN32
	auto res_value = result;
#else
	auto res_value = result.convert_to<mp::mpz_int>();
#endif
	return res_value;
}

biginteger convert_hex_to_biginteger(const string & input) {
	string s = "0x" + input;
	int index = s.find(" ");
	while (index != string::npos) {
		s = s.substr(0, index) + s.substr(index + 1);
		index = s.find(" ");
	}
	const char *str = s.c_str();
	return biginteger(str);
}

string hexStr(vector<byte> const & data)
{
	string res;
	boost::algorithm::hex(data.begin(), data.end(), back_inserter(res));
	boost::algorithm::to_lower(res);
	return res;
}

mt19937 get_seeded_random() {
	mt19937 mt;
	auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	mt.seed(seed);
	return mt;
}

mt19937_64 get_seeded_random64() {
	mt19937_64 mt;
	auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	mt.seed(seed);
	return mt;
}

void print_elapsed_ms(std::chrono::time_point<std::chrono::system_clock> start, string message) {
	auto end = std::chrono::system_clock::now();
	int elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	cout << message << " took: " << elapsed_ms << " ms" << endl;
}

void print_elapsed_micros(std::chrono::time_point<std::chrono::system_clock> start, string message) {
	auto end = std::chrono::system_clock::now();
	int elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	cout << message << " took: " << elapsed_ms << " microsseconds" << endl;
}

std::chrono::time_point<std::chrono::system_clock> scapi_now() {
	return chrono::system_clock::now();
}

biginteger getRandomInRange(biginteger min, biginteger max, std::mt19937 & random)
{
	boost::random::uniform_int_distribution<biginteger> ui(min, max);
	biginteger res = ui(random);
	return res;
}

biginteger getRandomPrime(int numBytes, int certainty, mt19937 & random) {
	biginteger p;
	biginteger max = mp::pow(biginteger(2),numBytes);
	
	do {
		p = getRandomInRange(0, max, random);
	} while (!isPrime(p, certainty));
	return p;
}

void print_byte_array(byte * arr, int len, string message)
{
	cout << message << endl;
	for (int i = 0; i < len; i++)
		cout << (int)arr[i] << ",";
	cout << endl;
}

bool isPrime(biginteger bi, int certainty) {
	auto prg = get_seeded_random();
	return (miller_rabin_test(bi, certainty, prg));
}