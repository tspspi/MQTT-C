#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include <mqtt.h>
#include "templates/posix_sockets.h"


static void publish_callback(void** unused, struct mqtt_response_publish *published);
static void* mqttThreadProcedure(void* client);

static void printUsage(int argc, const char**argv) {
	printf("Usage: %s HOST PORT TOPIC USERNAME PASSWORD IP MAC HOSTNAME\n", argv[0]);
}

static int socketOpen(const char* lpAddress, const char* lpPortString) {
	struct addrinfo hints = {0};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int fHandle = -1;
	int r;
	struct addrinfo* lpAdrInfoCurrent;
	struct addrinfo* lpServInfo;

	r = getaddrinfo(lpAddress, lpPortString, &hints, &lpServInfo);
	if(r != 0) {
		return -1;
	}

	for(lpAdrInfoCurrent = lpServInfo; lpAdrInfoCurrent != NULL; lpAdrInfoCurrent = lpAdrInfoCurrent->ai_next) {
		fHandle = socket(lpAdrInfoCurrent->ai_family, lpAdrInfoCurrent->ai_socktype, lpAdrInfoCurrent->ai_protocol);
		if(fHandle == -1) {
			continue;
		}

		r = connect(fHandle, lpAdrInfoCurrent->ai_addr, lpAdrInfoCurrent->ai_addrlen);
		if(r == -1) {
			close(fHandle);
			fHandle = -1;
			continue;
		}
		break;
	}

	freeaddrinfo(lpServInfo);

	if(fHandle != -1) {
		int blockMode = 1;
		ioctl(fHandle, FIONBIO, &blockMode);
	}

	return fHandle;
}

int main(int argc, const char **argv)  {
	const char* lpAddress;
	const char* lpPortString;
	const char* lpTopic;

	if(argc < 4+2+3) {
		printUsage(argc, argv);
		return -1;
	}

	lpAddress = argv[1];
	lpPortString = argv[2];
	lpTopic = argv[3];

	/* open the non-blocking TCP socket (connecting to the broker) */
	int socketHandle = socketOpen(lpAddress, lpPortString);

	if(socketHandle == -1) {
		fprintf(stderr, "Failed to open socket connection\n");
		return 1;
	}

	/* Create client */
	struct mqtt_client client;
	uint8_t lpBufSend[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
	uint8_t lpBufRecv[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */

	mqtt_init(&client, socketHandle, lpBufSend, sizeof(lpBufSend), lpBufRecv, sizeof(lpBufRecv), publish_callback);

	/* Create an anonymous session */
	const char* client_id = NULL;

	/* Ensure we have a clean session */
	uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;

	/* Send connection request to the broker. */
	mqtt_connect(&client, client_id, NULL, NULL, 0, argv[4], argv[5], connect_flags, 400);

	/* check that we don't have any errors */
	if (client.error != MQTT_OK) {
		fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
		close(socketHandle);
		return -1;
	}

	/* start a thread to refresh the client (handle egress and ingree client traffic) */
	pthread_t mqttThread;
	if(pthread_create(&mqttThread, NULL, mqttThreadProcedure, &client)) {
		printf("Failed to launch client thread\n");
		close(socketHandle);
		return -1;
	}

	char lpApplicationMessage[512];
	sprintf(lpApplicationMessage, "{ \"ip\" : \"%s\", \"mac\" : \"%s\", \"hostname\" : \"%s\" }", argv[6], argv[7], argv[8]);


	mqtt_publish(&client, lpTopic, lpApplicationMessage, strlen(lpApplicationMessage) + 1, MQTT_PUBLISH_QOS_0);
	/* check for errors */
	if (client.error != MQTT_OK) {
		fprintf(stderr, "Failed to publish message: %s\n", mqtt_error_str(client.error));
	}

	sleep(1);
	close(socketHandle);
	pthread_cancel(mqttThread);
	return 0;
}

static void publish_callback(void** unused, struct mqtt_response_publish *published) {
	/* Unused */
    return;
}

static void* mqttThreadProcedure(void* client)
{
	for(;;) {
		mqtt_sync((struct mqtt_client*) client);
		usleep(100000U);
	}
	return NULL;
}
