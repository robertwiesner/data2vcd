
#ifndef CTMPBUF_H
#define CTMPBUF_H 1

#include <vector>

class cTmpBuf {
	std::vector<char> buf;
public:
	cTmpBuf(size_t size) { buf.resize(size); }
	~cTmpBuf() { }
	char *getBuffer() { return buf.data(); }
	unsigned char* getUnsignedBuffer() { return reinterpret_cast<unsigned char *>(buf.data()); }
};
#endif
