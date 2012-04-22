// dpa framework - a collection of tools for differential power analysis
// Copyright (C) 2011  Garrett C. Smith
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#define BOOST_TEST_MODULE grostl

#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/test/included/unit_test.hpp>
#include "utility.h"
#include "grostl.h"

static struct {
    const char *msg, *hash; // msg is ASCII, hash is hex string
} grostl_hash_vectors[] = {
    { "abc", "f3c1bb19c048801326a7efbcf16e3d7887446249829c379e1840d1a3a1e7d4d2" },
    { "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "22c23b160e561f80924d44f2cc5974cd5a1d36f69324211861e63b9b6cb7974c" },
};

static struct {
    const char *msg, *chain, *out; // msg is ASCII, hash is hex string
} grostl_compress_vectors[] = {
    { "d21c0587d4e8b7a3f08bac13849d163a24c9d6271e91daa2ddd42fd4d4e73cd63f9edf76586996720ac10a35054d0cf450bbc880b9778bb4db3597cda6cfd669", "fa3a394ab16ff538fffcaaa8a430d0c4d362d6545b85011e50893d5f998b20962f89247800a1dcde3fc6c78c3e681584fe30fd4a3da6e195192ffa1e3dabb5c6", "a7a086de4a3098883a214e2e47dc30bd8596339c68132d9fdfbc4358afb02670b04cf86ff3c00e24a838d162d3f358039309e9176f56a1681a638615a0ba7367" },
    { "06aedbd33ca08dacd42d1183946bfa6ce6500c76bff1ba7cf53ecb48ed7efbe509fde4220819a00353adc8aa2a79e2dbe8efcaedc1bb24bf04405704f308262c", "4ab584bc3ce68150a40efd4937d26d4f0d85eff5be4e2bd923356d083699a63b9a893e22e94402bca7a411650bbcb8cf4f65a580fb6680e03bc63ac288a2482a", "27bbdee223021b6262e651707620308320af291bc45692feecde97f2cff3d9a06e446d735de2ff9cd1c7c58cceb048744bc2d9cc8c2cbfbd528988603f6d505f" },
    { "f578fd167de1aaf4a4e859bad6598777067eafee345977c48c7c9556fa7f1d2e045bbd84421fc7f2e51fbba453de2ca0f944d849f8fa2187d54a8fed0f48cebb", "e089a82ffb5ee5fde5687c8ff98f52605dc8d29c2bb6afef0499c15087f361512973f4bc70b79fc066d3ea845e75cc1e058c8838a2b36c0fc6d9f455cc227a7a", "54322130a9e82845b8c4d470afa1c0bdddaf251f9bf947d5c2355d6e645655ab09890427a0d837557dbccb6a3f0b714aabc1dbd37e836f961bc1edd256c8e2f4" },
    { "010c3c80e0b22f899d2ed6f65820b2d0688930795d20747b1d119790385eab83dde8aac6496469406833e9c87d1a3fa9ae18305e7ea74099574c7ce075477e7b", "235786ca1bffd7e1d9b5d19293839252ba0df4696c3ba51d49165644dabe65ac41a2cb17fe1c9f3e96c01a9f4a2f94757a2585d2b8f50b447c9b55e1662d5600", "ce8119f2bbba712735c96b0e589f618dc146655f5aef8fcfa79b3beb8623c19a575f6870cb417919b7369582f75961760a5402d12c8b4084fe05cbab3e1a52ea" },
    { "b221f0b0b7d8adb7f13f9cea7287f2027c58aa005771e60ea575e920828683f8b7663b896b9e461bbb1c37363030f1d8d236a3fe7965943a4addb7fcef62ab2a", "2ea9f3460e7f34e42214f1dd3105b379e059a4343d517862165789ced9f66f2dccee9946de6b96a8c125d9da5899b266803cfb2b5999ccf770b66b3093d245d3", "3ccc33209fa72f686773451e99da7eed0396638762f39f3caba4941bf307278947774ff9b8b0eacada5ef79ae0a406dec2c5bf0c4aa481d9dc99543fe97ab88f" },
    { "0ea8271a307a1dd6182d67b9e51f21735ab69b30431bc3e394df43b4bc684c72d84a2f687a151e6d7a905654b6c605c1a4cc29f94e9f994861725ac24c503696", "50db7e93a2987ccac1e19cbdc6de1fd41557e6a78f170b46282999e93c6b8dda1b1f79b57cc6bbbc0ecf5bb7ace581e5cfd19467ea5eb8c75e596006ef6c509a", "83e731789406416c1be138f08c24e2012fea3cdec76f570e295048c71381b5b5dc2a55552569c4278e3a30fdad71e57bf9a1119561352d545707f4599e594516" },
    { "5279de1888173fb5df7b677f990ac665ebc79fdf1e62ac89000169c7c00567619c83cc4bd7233fa17e453d0f541fd35a06e4b6f25d2db605feb7c9b9fd6385c9", "04bb0d4811110e46074006976a3e37e20677e823c5a703eef72f4cf77474de150fbd0a4a06f1a8b9af080d34aec9b645cb49961078a2cd871525a2f31e8fc942", "fe21ca30c87be5d36cbcc5544d4dc5462d77336a0ec18eaedde0bd9764c86957a02023c7f65f29ad2b45c4604ffb5e61dc1a6fac38b1958c0e877fe1aaffc7ae" },
    { "576bf3ed6ec8afbd37fc166a32ad8b80a62fcbc40af4fe1ec6e0c5798f553acab37208c5aabba5f126fe39c874bc8c02b2aa78058281e873b1115618a465a8b5", "57057240d83537bbfcf70c23d58005b2bfedb38d9b8e2a8fd0c633e4cc25d3df87f12f530a606dd3dcf0ef8e866bcffd905b61252098055f8bd7664416d556b5", "3c7c6a1d726623b32598622b1aec02dcc91c9f6aed6ca074a5d8f4e2aebf2b0ed8720b52de7f3dd55bb5d3a36abb02e226fc1219b2f78db7bbbd72ba46fee65c" },
    { "3acf0e4cfdac72c9950de3fe850e239f18a6ecd4986a2aed21dbe7af1dd4ac70c78939c82b68eb2e7e2517fb829ade920e59ecc1c3c9b0cb9e86aaa99103d40d", "4f2f4ea2251a3053c8f483b28811dee8d1fd713f03d836136197206245d9ad26ce97e4b5b07e2683d3b66cc7bc94bda0a2ac474967fea209a3103b6c39efa1af", "46103431d74970f5b6f35ce1bc857c96dd91302ce030e7e88eed589a5246bd4683b784efd5dd160036666e26da9fcd5871086378c7d9a9fb0a515d5b02ae63cc" },
    { "0a76e6dc98e909949960b5294c0283ba80c3e80cf80e46f6a269a645ace4931087db4882f58eaff69fae1e9398a646eeb0a4789732da8da51db4e2bfff5519df", "8c574614483af612624927fe541bf9100c31f6f640e9f62166e020dbdc488f21121a9b78543356d762609a59b0be0ebab71b8c3e0a006e95218a4a099fc173b8", "a3ad1cabed2f02529c885c80b94f522545f88481de6d08f5ef85f7dc1432bb8f88288360de5d4a4fbae5ff6ebb01f0dfdf15964c7fa726c2e3996dd1e35cc06f" },
};

// -----------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(grostl_test_hash)
{
    for (size_t i = 0; i < NUM_ELEMENTS(grostl_hash_vectors); i++) {
        const char *msg = grostl_hash_vectors[i].msg;
        std::vector<uint8_t> out, in(msg, msg + strlen(msg));

        grostl::hash(in, out);
        
        const std::string hash_str = util::btoa(out);
        const std::string hash_vec = grostl_hash_vectors[i].hash;

        BOOST_CHECK( hash_str == boost::to_upper_copy(hash_vec) );
    }
}

// -----------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(grostl_test_compress)
{
    uint8_t msg[64], chain[64], out[64];

    for (size_t i = 0; i < NUM_ELEMENTS(grostl_compress_vectors); i++) {
        BOOST_CHECK( util::atob(grostl_compress_vectors[i].msg,   msg,   64) );
        BOOST_CHECK( util::atob(grostl_compress_vectors[i].chain, chain, 64) );

        grostl::compress(msg, chain, out);

        const std::string out_str = util::btoa(out, 64);
        const std::string out_vec = grostl_compress_vectors[i].out;

        BOOST_CHECK( out_str.size() == 128 );
        BOOST_CHECK( out_str == boost::to_upper_copy(out_vec) );
    }
}

// -----------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(grostl_test_compress_masked)
{
    uint8_t msg[64], chain[64], out[64], imask[64], omask[64];

    for (size_t i = 0; i < NUM_ELEMENTS(grostl_compress_vectors); i++) {
        BOOST_CHECK( util::atob(grostl_compress_vectors[i].msg,   msg,   64) );
        BOOST_CHECK( util::atob(grostl_compress_vectors[i].chain, chain, 64) );

        for (int j = 0; j < 64; j++) {
            imask[j] = rand() % 256;
            omask[j] = rand() % 256;
        }

        grostl::compress(msg, chain, out, imask, omask);

        const std::string out_str = util::btoa(out, 64);
        const std::string out_vec = grostl_compress_vectors[i].out;

        BOOST_CHECK( out_str.size() == 128 );
        BOOST_CHECK( out_str == boost::to_upper_copy(out_vec) );
    }
}

