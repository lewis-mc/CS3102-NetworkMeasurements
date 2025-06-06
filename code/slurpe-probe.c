#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include "UdpSocket.h"
#include "byteorder64.h"

#define G_SRV_PORT ((uint16_t)23220)
#define NUM_PROBES 1200 // 20 minutes worth
#define RESPONSE_TIMEOUT 1 

volatile sig_atomic_t send_probe_flag = 0;

// Packet structure
typedef struct {

    uint32_t seq_number;
    uint32_t payload_size;
    uint64_t timestamp;

} probe_packet;

// Initialises all functions
void setup_signal_handling();
void setup_timer();
void send_probe(UdpSocket_t *localUdp, UdpSocket_t *remoteUdp, uint32_t seq, size_t payload_size);
void receive_echo(UdpSocket_t *localUdp, UdpSocket_t *remoteUdp, int sockfd, size_t payload_size);
void process_received_echo(probe_packet *recvPacket, size_t payload_size);

// Signal Handler for SIGALRM
void alarm_handler(int signum) {
    send_probe_flag = 1;
}

int main(int argc, char *argv[]) {

    // Handles command line arguements
    if (argc != 3) {
        printf("Usage: ./slurpe-probe <target_address> <payload_size>\n");
        return 1;
    }

    
    size_t payload_size = atoi(argv[2]);

    setup_signal_handling();
    setup_timer();

    // Sets up the local UDP socket
    UdpSocket_t *localUdp = setupUdpSocket_t(NULL, G_SRV_PORT);
    if (!localUdp || openUdp(localUdp) < 0) {
        perror("Failed to open local UDP socket");
        return 1;
    }

    // Sets up the local UDP socket
    UdpSocket_t *remoteUdp = setupUdpSocket_t(argv[1], G_SRV_PORT);
    if (!remoteUdp) {
        perror("Failed to setup remote UDP for slurpe-5");
        closeUdp(localUdp);
        return 1;
    }


    // Initial output with experiment information
    printf("\nstart: packets_sent: %u payload_size: %zu\n", NUM_PROBES, payload_size);

    int sockfd = localUdp->sd;
    uint32_t seq = 1;


    // while loop for all probes in the experiment
    while (seq <= NUM_PROBES) {

        // sends a probe packet if the flag is set
        if (send_probe_flag) {
            send_probe(localUdp, remoteUdp, seq, payload_size);
            send_probe_flag = 0; // probe flag is reset
            seq++;
        }

        receive_echo(localUdp, remoteUdp, sockfd, payload_size);
    }

    // Cleanup
    closeUdp(localUdp);
    closeUdp(remoteUdp);
    return 0;
}

void setup_signal_handling() {

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    sigaction(SIGALRM, &sa, NULL);

}

void setup_timer() {

    struct itimerval timer;
    timer.it_value.tv_sec = 1; // First alarm after 1 second
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1; // Repeat alarm every 1 second
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

}

// Sends the probe packet with the correct payload size
void send_probe(UdpSocket_t *localUdp, UdpSocket_t *remoteUdp, uint32_t seq, size_t payload_size) {

    size_t packet_size = sizeof(probe_packet) + payload_size; // determines the packet size
    probe_packet *packet = malloc(packet_size); // allocates memory for the packet
    
    if (!packet) {
        perror("Failed to allocate memory for probe packet");
        return;
    }

    // sets packet values
    packet->seq_number = htonl(seq);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    packet->timestamp = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
    packet->timestamp = hton64(packet->timestamp);
    packet->payload_size = htonl(payload_size);
    
    memset(((uint8_t*)packet) + sizeof(probe_packet), 0xAA, payload_size); 

    UdpBuffer_t buffer;
    buffer.bytes = (uint8_t *)packet;
    buffer.n = packet_size;

    if (sendUdp(localUdp, remoteUdp, &buffer) < 0) {
        fprintf(stderr, "Failed to send packet %u\n", seq);
    }

    free(packet);
}


// handles the receiving of all packets
void receive_echo(UdpSocket_t *localUdp, UdpSocket_t *remoteUdp, int sockfd, size_t payload_size) {

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    struct timeval timeout;
    timeout.tv_sec = RESPONSE_TIMEOUT;
    timeout.tv_usec = 0;

    sigset_t emptyset;
    sigemptyset(&emptyset);

    if (pselect(sockfd + 1, &readfds, NULL, NULL, &timeout, &emptyset) > 0) {

        if (FD_ISSET(sockfd, &readfds)) {
            // Dynamically adjust buffer size for received packet based on expected max packet size
            size_t max_packet_size = sizeof(probe_packet) + payload_size;
            uint8_t *buffer = malloc(max_packet_size);
            if (!buffer) {
                perror("Failed to allocate memory for receiving buffer");
                return;
            }

            UdpBuffer_t recvBuffer;
            recvBuffer.bytes = buffer;
            recvBuffer.n = max_packet_size;

            if (recvUdp(localUdp, remoteUdp, &recvBuffer) > 0) {
                process_received_echo((probe_packet*)recvBuffer.bytes, recvBuffer.n - sizeof(probe_packet));
            } else if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Error receiving packet");
            }

            free(buffer);
        }
    }
}


// processes the received probe packet and prints the output, so it can be used in the experiment
void process_received_echo(probe_packet *recvPacket, size_t payload_size) {

    recvPacket->seq_number = ntohl(recvPacket->seq_number);
    recvPacket->payload_size = ntohl(recvPacket->payload_size);
    recvPacket->timestamp = ntoh64(recvPacket->timestamp);
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t now_ = (uint64_t)now.tv_sec * 1000000 + now.tv_usec;
    printf("seq: %u timestamp: %lu rtt: %0.3f\n", recvPacket->seq_number, recvPacket->timestamp, (now_ - recvPacket->timestamp) / 1000.0);

}
