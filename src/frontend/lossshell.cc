/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <vector>
#include <string>
#include <iostream>

#include <getopt.h>

#include "loss_queue.hh"
#include "util.hh"
#include "ezio.hh"
#include "packetshell.cc"

using namespace std;

void usage( const string & program_name )
{
    throw runtime_error( "Usage: " + program_name + " IID|bursty uplink|downlink RATE1 RATE2 [COMMAND...]" );
}

int main( int argc, char *argv[] )
{
    try {
        /* clear environment while running as root */
        char ** const user_environment = environ;
        environ = nullptr;

        check_requirements( argc, argv );

        if ( argc < 4 ) {
            usage( argv[ 0 ] );
        }

        const string loss_type = argv[1];
        int expected_args = 4;
        
        /* parse first rate as loss rate/probability of leaving no loss state */
        const double loss_rate = myatof( argv[ 3 ] );
        if ( (0 <= loss_rate) and (loss_rate <= 1) ) {
            /* do nothing */
        } else {
            cerr << "Error: IID/bursty loss rate/prob must be between 0 and 1." << endl;
            usage( argv[ 0 ] );
        }

        double uplink_loss = 0, downlink_loss = 0;

        const string link = argv[ 2 ];
        if ( link == "uplink" ) {
            uplink_loss = loss_rate;
        } else if ( link == "downlink" ) {
            downlink_loss = loss_rate;
        } else {
            usage( argv[ 0 ] );
        }

        /* parse second probability as probability of leaving loss state 
         * if it is bursty */
        double leave_loss_prob = 0;
        if ( loss_type == "IID" ) {
            expected_args = 4;
        } else {
            expected_args = 5;
            if ( argc < 5 ) {
                usage( argv[ 0 ] );
            }
            
            const double leave_loss_prob = myatof( argv[ 4 ] );
            if ( (0 <= leave_loss_prob) and (leave_loss_prob <= 1) ) {
                /* do nothing */
            } else {
                cerr << "Error: bursty loss prob must be between 0 and 1." << endl;
                usage( argv[ 0 ] );
            }
        }

        vector<string> command;

        if ( argc == expected_args ) {
            command.push_back( shell_path() );
        } else {
            for ( int i = expected_args; i < argc; i++ ) {
                command.push_back( argv[ i ] );
            }
        }

        string shell_prefix = "[loss ";
        if ( link == "uplink" ) {
            shell_prefix += "up=";
        } else {
            shell_prefix += "down=";
        }
        shell_prefix += argv[ 3 ];


        if ( loss_type == "IID" ) {
            shell_prefix += "] ";
            PacketShell<IIDLoss> loss_app( "loss", user_environment );

            loss_app.start_uplink( shell_prefix,
                                   command,
                                   uplink_loss );
            loss_app.start_downlink( downlink_loss );
            return loss_app.wait_for_exit();

        } else if ( loss_type == "bursty") {
            shell_prefix += " " + argv[4];
            shell_prefix += "] ";

            PacketShell<BurstyLoss> loss_app( "loss", user_environment );

            loss_app.start_uplink( shell_prefix,
                                   command,
                                   leave_loss_prob, uplink_loss );
            loss_app.start_downlink( leave_loss_prob, downlink_loss );
            return loss_app.wait_for_exit();

        }

        else {
            usage( argv[ 0 ] );
        }

    } catch ( const exception & e ) {
        print_exception( e );
        return EXIT_FAILURE;
    }
}
