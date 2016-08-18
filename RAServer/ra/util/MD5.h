#ifndef RA_MD5_H
#define RA_MD5_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(util);

/** Calculate Ms5 for a byte stream,
* result is stored in md5[16]
*
* @param poolIn Input data
* @param inputBytesNum Length of input data
* @param md5[16] A 128-bit pool for storing md5
*/
void DoMd5(const uint8_t* poolIn, const uint64_t inputBytesNum, uint8_t md5[16]);

/** check correctness of a Md5-calculated byte stream
*
* @param poolIn Input data
* @param inputBytesNum Length of input data
* @param md5[16] A 128-bit md5 value for checking
*
* @return true if no error detected, false if error detected
*/
bool CheckMd5(const uint8_t* poolIn, const uint64_t inputBytesNum, const uint8_t md5[16]);

/** @brief Md5Stream can receive byte stream as input,
 *  use Put() to put sequential streams,
 *  then call Get() to get md5 for all the input streams
 */
class Md5Stream
{
public:
    /** @brief Constructor
     */
    Md5Stream();

    /** @brief Give an input pool into this class
     *
     *  @param poolIn The input pool
     *  @param inputBytesNum The input bytes number in poolIn
     */
    void Put(const uint8_t* poolIn, const uint64_t inputBytesNum);

    /** @brief Fill the given hash array with md5 value
     *
     *  @param hash The array to get md5 value
     */
    void Get(uint8_t hash[16]);

private:
    /** @brief Initailize class members
     */
    void Initailize();

    bool mLittleEndian;     /// true if litte endian, false if big endian
    uint8_t mBuf[64];       /// hold remained input stream
    uint64_t mBufPosition;  /// indicate the position of stream end in buf
    uint32_t mH[4];         /// hold inter result
    uint64_t mTotalBytes;   /// total bytes
}; /// class Md5Stream

RA_END_NAMESPACE(util);

#endif ///RA_MD5_H
