/*
 * MqttHandler.cpp
 *
 *  Created on: 29.11.2016
 *      Author: bensen
 */

#include "MqttHandler.h"
#include <string>
#include <queue>

MqttHandler::MqttHandler() {
	// TODO Auto-generated constructor stub

}

MqttHandler::~MqttHandler() {
	// TODO Auto-generated destructor stub
}


void delivered(void *context, MQTTClient_deliveryToken dt){
	printf("Message with token value %d delivery confirmed\n", dt);
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message){
	int i;
	char* payloadptr;
	MqttPckg pack;
	pack.topic=topicName;
	pack.mssg=" ";
	payloadptr = (char*) message->payload;
	for(i=0; i<message->payloadlen; i++){
		pack.mssg+=*payloadptr++;
	}
	inQueue.push(pack);
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}
void connlost(void *context, char *cause){
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
	int i = 0;
	disconnectDestroy(subClient);
	disconnectDestroy(pubClient);
	while(i<10){
		//	nanosleep(100);
			i++;
		try{
			subClient= initSubscriber( (char*)"#", (char*) "s2" );
			pubClient= initPublisher( (char*) "p1" );
			break;
		}
		catch(int e){
			printf("%d",e);
		}
	}

}
MQTTClient initSubscriber(char* topic, char* subClient){
	int rc;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_create(&client, ADDRESS, SUBCLIENT, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	//		conn_opts.username = USER;
	//		conn_opts.password = PW;


	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		throw(-1);
	}
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
			"Press Q<Enter> to quit\n\n", topic, SUBCLIENT, QOS);
	MQTTClient_subscribe(client, topic, QOS);
	return client;
}

MQTTClient initPublisher(char* pubClient){
	int rc;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_create(&client, ADDRESS, "pubClient", MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	//	conn_opts.username = USER;
	//	conn_opts.password = PW;
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		throw(-1);
	}
	return client;
}
void disconnectDestroy(MQTTClient client){
	MQTTClient_disconnect(client, 100);
	MQTTClient_destroy(&client);
}
void printInQueue(){
	while (!inQueue.empty()){
		std::string ms= inQueue.front().mssg;
		std::string to= inQueue.front().topic;
		inQueue.pop();
		std::cout<<to+" - "+ ms<<std::endl;
	}
	std::cout<<"End"<<std::endl;
}

void publish(MqttPckg p){
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	const char* m= p.mssg.c_str();
	const char* t= p.topic.c_str();
	pubmsg.payload = (void*) m;
	pubmsg.payloadlen = strlen((char*)pubmsg.payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_deliveryToken token;
	const char* topicC= t;
	MQTTClient_publishMessage(pubClient, topicC, &pubmsg, &token);
	//	std::cout<<p.mssg +"  "+p.topic<<std::endl;
}

void publishOutQueue(){
	while(!outQueue.empty()){
		publish(outQueue.front());
		outQueue.pop();
	}
}
MqttPckg buildMqttPckg(int mPckgType, int type, int mac, int id, std::string payload ){
	MqttPckg pckg;

	switch (mPckgType){
	//actual
	case (1): break;
	//history
	case (2): break;
	//battery:
	case (3): break;
	//case (4): break;

	}
	return pckg;
}

int main (){
	//start subscriber by passing topic and an identifier
			subClient= initSubscriber( (char*)"#", (char*) "s2" );
	//start publisher by passing an identifier
			pubClient= initPublisher( (char*) "p1" );

	//create Packages (stringTopic and stringMessage) and push them in the outQueue for testing purposes
			MqttPckg a("t1", "m1");
			MqttPckg b("t2", "m2");
			MqttPckg c("t3", "m3");
			outQueue.push(a);
			outQueue.push(b);
			outQueue.push(c);

	//publishing Whatever is in the outQueue
			publishOutQueue();

	//let Program run until pressed 'Q' or 'q'
			int ch;
			do{
				ch = getchar();
			} while (ch!='Q' && ch != 'q');

	//print Messages arrived and close Subscriber
			printInQueue();

			//	disconnectDestroy(aSubClient);
			disconnectDestroy(subClient);
			disconnectDestroy(pubClient);
	return 0;
}
