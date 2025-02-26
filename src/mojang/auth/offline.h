#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <vector>
#include <openssl/evp.h>

namespace mojang::auth
{
    struct UserInfo {
        std::string access_token;
        std::string session_token;
        std::string username;
        std::string uuid;
    };

    class Offline {
    public:
        UserInfo Login( const std::string &username ) {
            UserInfo user_info;
            user_info.username = username;
            user_info.uuid = GenerateUUID( username );

            // i dont think servers really check access and sesion token since theres no way to check if it is real or wtv
            user_info.access_token = GenerateRandomString( 32 );
            user_info.session_token = GenerateRandomString( 32 );

            return user_info;
        }

    private:
        std::vector<unsigned char> ComputeMD5( const std::string& input ) {
            EVP_MD_CTX * ctx = EVP_MD_CTX_new( );
            if ( !ctx ) {
                throw std::runtime_error( "Failed to create OpenSSL EVP_MD_CTX" );
            }

            if ( EVP_DigestInit_ex( ctx, EVP_md5( ), NULL ) != 1 ||
                 EVP_DigestUpdate( ctx, input.c_str( ), input.size( ) ) != 1 ) {
                EVP_MD_CTX_free( ctx );
                throw std::runtime_error( "OpenSSL MD5 computation failed" );
            }

            std::vector<unsigned char> digest( EVP_MD_size( EVP_md5( ) ) );
            unsigned int digest_length = 0;

            if ( EVP_DigestFinal_ex( ctx, digest.data( ), &digest_length ) != 1 ) {
                EVP_MD_CTX_free( ctx );
                throw std::runtime_error( "OpenSSL MD5 finalization failed" );
            }

            EVP_MD_CTX_free( ctx );
            return digest;
        }

        std::string GenerateUUID( const std::string& username ) {
            std::string offline_player_str = "OfflinePlayer:" + username;
            std::vector<unsigned char> hash = ComputeMD5( offline_player_str );

            if ( hash.size( ) < 16 ) {
                throw std::runtime_error( "MD5 digest too short" );
            }

            uint64_t msb = 0, lsb = 0;
            for ( int i = 0; i < 8; i++ ) msb = ( msb << 8 ) | hash[ i ];
            for ( int i = 8; i < 16; i++ ) lsb = ( lsb << 8 ) | hash[ i ];

            msb &= 0xFFFFFFFFFFFF0FFFULL;
            msb |= 0x0000000000003000ULL;
            lsb &= 0x3FFFFFFFFFFFFFFFULL;
            lsb |= 0x8000000000000000ULL;

            std::stringstream uuid;
            uuid << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ( msb >> 32 );
            uuid << "-";
            uuid << std::hex << std::setw( 4 ) << std::setfill( '0' ) << ( ( msb >> 16 ) & 0xFFFF );
            uuid << "-";
            uuid << std::hex << std::setw( 4 ) << std::setfill( '0' ) << ( msb & 0xFFFF );
            uuid << "-";
            uuid << std::hex << std::setw( 4 ) << std::setfill( '0' ) << ( ( lsb >> 48 ) & 0xFFFF );
            uuid << "-";
            uuid << std::hex << std::setw( 12 ) << std::setfill( '0' ) << ( lsb & 0xFFFFFFFFFFFFULL );

            return uuid.str( );
        }

        std::string GenerateRandomString( std::size_t length ) {
            static const char characters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            std::random_device rd;
            std::mt19937 gen( rd( ) );
            std::uniform_int_distribution<> dis( 0, sizeof( characters ) - 2 );

            std::string random_string;
            for ( std::size_t i = 0; i < length; ++i ) {
                random_string += characters[ dis( gen ) ];
            }

            return random_string;
        }
    };
}
