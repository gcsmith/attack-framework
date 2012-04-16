#define BOOST_TEST_MODULE aes

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/test/included/unit_test.hpp>
#include "utility.h"
#include "aes.h"

#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

static struct {
    const char *key, *pt, *ct;
} aes_test_vectors[] = {
    { "2b7e151628aed2a6abf7158809cf4f3c", "6bc1bee22e409f96e93d7e117393172a", "3ad77bb40d7a3660a89ecaf32466ef97" },
    { "2b7e151628aed2a6abf7158809cf4f3c", "ae2d8a571e03ac9c9eb76fac45af8e51", "f5d3d58503b9699de785895a96fdbaaf" },
    { "2b7e151628aed2a6abf7158809cf4f3c", "30c81c46a35ce411e5fbc1191a0a52ef", "43b1cd7f598ece23881b00e3ed030688" },
    { "2b7e151628aed2a6abf7158809cf4f3c", "f69f2445df4f9b17ad2b417be66c3710", "7b0c785e27e8ad3f8223207104725dd4" },
    { "E8E9EAEBEDEEEFF0F2F3F4F5F7F8F9FA", "014BAF2278A69D331D5180103643E99A", "6743C3D1519AB4F2CD9A78AB09A511BD" },
    { "000102030405060708090a0b0c0d0e0f", "00112233445566778899aabbccddeeff", "69c4e0d86a7b0430d8cdb78070b4c55a" },
};

// test encryption with no masking countermeasure
BOOST_AUTO_TEST_CASE(aes_test_encrypt)
{
    uint8_t pt[16], out[16], sk[16 * 11];

    for (size_t i = 0; i < NUM_ELEMENTS(aes_test_vectors); i++) {
        BOOST_CHECK( util::atob(aes_test_vectors[i].key, sk, 16) );
        BOOST_CHECK( util::atob(aes_test_vectors[i].pt,  pt, 16) );

        aes::key_schedule(sk, 10);
        aes::encrypt(pt, sk, out);
        
        const std::string ct_str = util::btoa(out, 16);
        const std::string ct_vec = aes_test_vectors[i].ct;

        BOOST_CHECK( ct_str.size() == 32 );
        BOOST_CHECK( ct_str == boost::to_upper_copy(ct_vec) );
    }
}

// test decryption with no masking countermeasure
BOOST_AUTO_TEST_CASE(aes_test_decrypt)
{
    uint8_t ct[16], out[16], sk[16 * 11];

    for (size_t i = 0; i < NUM_ELEMENTS(aes_test_vectors); i++) {
        BOOST_CHECK( util::atob(aes_test_vectors[i].key, sk, 16) );
        BOOST_CHECK( util::atob(aes_test_vectors[i].ct,  ct, 16) );

        aes::key_schedule(sk, 10);
        aes::decrypt(ct, sk, out);
        
        const std::string pt_str = util::btoa(out, 16);
        const std::string pt_vec = aes_test_vectors[i].pt;

        BOOST_CHECK( pt_str.size() == 32 );
        BOOST_CHECK( pt_str == boost::to_upper_copy(pt_vec) );
    }
}

// test encryption with simple one byte mask, where imask == omask
BOOST_AUTO_TEST_CASE(aes_test_encrypt_simple_mask_1)
{
    uint8_t pt[16], out[16], sk[16 * 11];

    for (size_t i = 0; i < NUM_ELEMENTS(aes_test_vectors); i++) {
        BOOST_CHECK( util::atob(aes_test_vectors[i].key, sk, 16) );
        BOOST_CHECK( util::atob(aes_test_vectors[i].pt,  pt, 16) );

        uint8_t mask = rand() % 256;
        aes::key_schedule(sk, 10);
        aes::encrypt_mask(pt, sk, out, mask, mask);
        
        const std::string ct_str = util::btoa(out, 16);
        const std::string ct_vec = aes_test_vectors[i].ct;

        BOOST_CHECK( ct_str.size() == 32 );
        BOOST_CHECK( ct_str == boost::to_upper_copy(ct_vec) );
    }
}

// test decryption with simple one byte mask, where imask == omask
BOOST_AUTO_TEST_CASE(aes_test_decrypt_simple_mask_1)
{
    uint8_t ct[16], out[16], sk[16 * 11];

    for (size_t i = 0; i < NUM_ELEMENTS(aes_test_vectors); i++) {
        BOOST_CHECK( util::atob(aes_test_vectors[i].key, sk, 16) );
        BOOST_CHECK( util::atob(aes_test_vectors[i].ct,  ct, 16) );

        uint8_t mask = rand() % 256;
        aes::key_schedule(sk, 10);
        aes::decrypt_mask(ct, sk, out, mask, mask);
        
        const std::string pt_str = util::btoa(out, 16);
        const std::string pt_vec = aes_test_vectors[i].pt;

        BOOST_CHECK( pt_str.size() == 32 );
        BOOST_CHECK( pt_str == boost::to_upper_copy(pt_vec) );
    }
}

// test encryption with simple one byte mask, where imask != omask
BOOST_AUTO_TEST_CASE(aes_test_encrypt_simple_mask_2)
{
    uint8_t pt[16], out[16], sk[16 * 11];

    for (size_t i = 0; i < NUM_ELEMENTS(aes_test_vectors); i++) {
        BOOST_CHECK( util::atob(aes_test_vectors[i].key, sk, 16) );
        BOOST_CHECK( util::atob(aes_test_vectors[i].pt,  pt, 16) );

        uint8_t imask = rand() % 256;
        uint8_t omask = rand() % 256;
        aes::key_schedule(sk, 10);
        aes::encrypt_mask(pt, sk, out, imask, omask);
        
        const std::string ct_str = util::btoa(out, 16);
        const std::string ct_vec = aes_test_vectors[i].ct;

        BOOST_CHECK( ct_str.size() == 32 );
        BOOST_CHECK( ct_str == boost::to_upper_copy(ct_vec) );
    }
}

// test decryption with simple one byte mask, where imask != omask
BOOST_AUTO_TEST_CASE(aes_test_decrypt_simple_mask_2)
{
    uint8_t ct[16], out[16], sk[16 * 11];

    for (size_t i = 0; i < NUM_ELEMENTS(aes_test_vectors); i++) {
        BOOST_CHECK( util::atob(aes_test_vectors[i].key, sk, 16) );
        BOOST_CHECK( util::atob(aes_test_vectors[i].ct,  ct, 16) );

        uint8_t imask = rand() % 256;
        uint8_t omask = rand() % 256;
        aes::key_schedule(sk, 10);
        aes::decrypt_mask(ct, sk, out, imask, omask);
        
        const std::string pt_str = util::btoa(out, 16);
        const std::string pt_vec = aes_test_vectors[i].pt;

        BOOST_CHECK( pt_str.size() == 32 );
        BOOST_CHECK( pt_str == boost::to_upper_copy(pt_vec) );
    }
}

