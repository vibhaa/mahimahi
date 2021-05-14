/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef LOSS_QUEUE_HH
#define LOSS_QUEUE_HH

#include <queue>
#include <cstdint>
#include <string>
#include <random>
#include <iostream>

#include "file_descriptor.hh"

class LossQueue
{
private:
    std::queue<std::string> packet_queue_ {};

    virtual bool drop_packet( const std::string & packet ) = 0;

protected:
    std::default_random_engine prng_;

public:
    LossQueue();
    virtual ~LossQueue() {}

    void read_packet( const std::string & contents );

    void write_packets( FileDescriptor & fd );

    unsigned int wait_time( void );

    bool pending_output( void ) const { return not packet_queue_.empty(); }

    static bool finished( void ) { return false; }
};

class IIDLoss : public LossQueue
{
private:
    std::bernoulli_distribution drop_dist_;

    bool drop_packet( const std::string & packet ) override;

public:
    IIDLoss( const double loss_rate ) : drop_dist_( loss_rate ) {}
};

class BurstyLoss : public LossQueue
{
private:
    std::uniform_real_distribution<> uniform_dist_;
    
    const double probability_leave_loss_;
    const double probability_leave_no_loss_;
    
    bool in_loss_state_;

    bool drop_packet( const std::string & packet ) override;

public:
    BurstyLoss( const double prob_leave_loss, const double prob_leave_no_loss) : 
        uniform_dist_( 0.0, 1.0 ),
        probability_leave_loss_( prob_leave_loss ),
        probability_leave_no_loss_( prob_leave_no_loss ),
        in_loss_state_( false ) {
            std::cerr << "bursty loss link P(leave loss) " << prob_leave_loss
                << " P( leave no loss) " << prob_leave_no_loss << std::endl; }
};

class SwitchingLink : public LossQueue
{
private:
    bool link_is_on_;
    std::exponential_distribution<> on_process_;
    std::exponential_distribution<> off_process_;

    uint64_t next_switch_time_;

    void calculate_next_switch_time( void );

    bool drop_packet( const std::string & packet ) override;

public:
    SwitchingLink( const double mean_on_time_, const double mean_off_time );

    unsigned int wait_time( void );
};

#endif /* LOSS_QUEUE_HH */
