// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message


void KernelFn()
{
	int i=0;
	 PacketHeader outPktHdr, inPktHdr;
	 MailHeader outMailHdr, inMailHdr;
	char buffer[40];
	int NoOfUpComingMsg=0;
	int flag0=0;
	int flag1=0;
	int flag2=0;
	int flag3=0;
	int flag4=0;
	int j=0;
	char Delimiter=';';
	int SentMsgAck0[5];
	int SentMsgAck1[5];
	int SentMsgAck2[5];
	int SentMsgAck3[5];
	int SentMsgAck4[5];
	char SrvBuffer[40];
	char *msg=new char[10];
	

	for(i=0;i<5;i++)            /*initialization*/
	{
		 SentMsgAck0[i]=-1;
		 SentMsgAck1[i]=-1;
		 SentMsgAck2[i]=-1;
		 SentMsgAck3[i]=-1;
		 SentMsgAck4[i]=-1;
	}
	
	   Delay(7);
    char *data=new char[10];
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
		j=0;
		for(i=0;i<2;i++)
		{
		
				 if(i==myClientID)
				 {
						continue;
				 }
					outPktHdr.to = i;		
					outMailHdr.to = 0;
					outMailHdr.from = 0;

				 
				// Send the first message
				for(j=0;j<5;j++)
				{
				sprintf(data,"%s%c%d","tmsg",Delimiter,j);
				outMailHdr.length = strlen(data) + 1;
				//printf("sending message %s to %d client %d box\n", data,outPktHdr.to,outMailHdr.from);
				bool success = postOffice->Send1(outPktHdr, outMailHdr, data);
				if ( !success ) {
					printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					}
				}		
		}
	

	
		while(true)                        /*continuously receive, either ack's or messages from other threads*/
		{
		  postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
			DEBUG('q',"Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from); 
			//Unpack the message ==buffer
			char* myData = strtok(buffer,";");
			char *tempmsgNum=strtok(NULL,";"); 
			int msgNum=atoi(tempmsgNum);
			if(strcmp(myData,"ack")==0)              /*checking whether its ack*/
			{
				//Remove the corresponding message from this client
	
				char *myMsg=new char[10]; //when removed from Q
				char *BufMsg=new char[10];
				int CounterBreak=0;
				int MID;
				int CID;

				while(!MsgQ->IsEmpty())               /*remove the message from the send queue if ack is recieved */
				{
					myMsg=(char*)MsgQ->Remove();

					strcpy(BufMsg,myMsg);
					//printf("REMOVED MSG FROM Q-->%s\n",myMsg);
					char *tempMID=strtok(BufMsg,";");
					char *tempCID=strtok(NULL,";");
					MID=atoi(tempMID);
				  CID=atoi(tempCID); //change
					DEBUG('q',"(Rx)msgNum%d inPktHdr.from%d (MsgQ)MID%d CID%d \n",msgNum,inPktHdr.from,MID,CID); 
					if(msgNum==MID && inPktHdr.from==CID)
					{
					  //printf("Ack Rx for Seq Number %d from ClientID %d\n",msgNum,inPktHdr.from);
						//printf("REMOVED MSG FROM Q-->%s\n",myMsg);
					 
						break;                                /*Successfully removed*/
					}
					else
					{
						MsgQ->Append((void *)myMsg);          /*otherwise append it back*/
						if(CounterBreak==5)
						{
							DEBUG('q',"BREAKING FOR DUPLICATE ACK \n",msgNum,inPktHdr.from,MID,CID); 
							break;
						}
						CounterBreak++;
					}
				}
			}
			else                                      /*if it is a an actual msg*/
			{
				/*I got a message from other kernel fn, if it is in order i need to print it out, if it is out of order
					 but i have akcked alreday, send ack again*/ 

				     if(inPktHdr.from==0)        /*if its client id 0 and its not current thread client id*/
							{
								if(flag0==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck0[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag0++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we send????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck0[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
										//	printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							
							
							if(inPktHdr.from==1)        /*if its client id 0 and its not current thread client id*/
							{
								if(flag1==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck1[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag1++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we send????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck1[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
										//	printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							
						
							
						
			}
				
			}
		}
	
//For clients >2
void KernelFn2()
{
	int i=0;
	 PacketHeader outPktHdr, inPktHdr;
	 MailHeader outMailHdr, inMailHdr;
	char buffer[40];
	int NoOfUpComingMsg=0;
	int flag0=0;
	int flag1=0;
	int flag2=0;
	int flag3=0;
	int flag4=0;
	char Delimiter=';';
	int SentMsgAck0[5];
	int SentMsgAck1[5];
	int SentMsgAck2[5];
	int SentMsgAck3[5];
	int SentMsgAck4[5];
	char SrvBuffer[40];
	char *msg=new char[10];
	

	for(i=0;i<5;i++)            /*initialization*/
	{
		 SentMsgAck0[i]=-1;
		 SentMsgAck1[i]=-1;
		 SentMsgAck2[i]=-1;
		 SentMsgAck3[i]=-1;
		 SentMsgAck4[i]=-1;
	}
	
	
	
	
		Delay(10);
    char *data=new char[10];
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    
	int j=0;
		for(int i=0;i<5;i++)
		{
			// Send the first message
			if(i!=myClientID)
			{
				outPktHdr.to = i;
				outMailHdr.to = 0;
				outMailHdr.from = 0;
				for(j=0;j<5;j++)
				{
					sprintf(data,"%s%c%d","tmsg",Delimiter,j);
					outMailHdr.length = strlen(data) + 1;
					//printf("sending message %s to %d client %d box\n", data,outPktHdr.to,outMailHdr.from);
					bool success = postOffice->Send1(outPktHdr, outMailHdr, data);
					if ( !success ) {
						printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
					}		
				}
				Delay(4);
			}
			
		}
		
	 

	
		while(true)                        /*continuously receive, either ack's or messages from other threads*/
		{
		  postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
			DEBUG('q',"Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from); 
			//Unpack the message ==buffer
			char* myData = strtok(buffer,";");
			char *tempmsgNum=strtok(NULL,";"); 
			int msgNum=atoi(tempmsgNum);

			if(strcmp(myData,"ack")==0)              /*checking whether its ack*/
			{
				//Remove the corresponding message from this client
	
				char *myMsg=new char[10]; //when removed from Q
				char *BufMsg=new char[10];
				int MID;
				int CID;
				int CounterBreak=0;
				while(!MsgQ->IsEmpty())               /*remove the message from the send queue if ack is recieved */
				{
					myMsg=(char*)MsgQ->Remove();
					strcpy(BufMsg,myMsg);
					//printf("REMOVED MSG FROM Q-->%s\n",myMsg);
					char * tempMID=strtok(BufMsg,";");
					char *tempCID=strtok(NULL,";");
						MID=atoi(tempMID);
				  	CID=atoi(tempCID); //change
					if(msgNum==MID && inPktHdr.from==CID)
					{
					 // printf("Ack Rx for Seq Number %d from ClientID %d\n",msgNum,inPktHdr.from);
						//printf("REMOVED MSG FROM Q-->%s\n",myMsg);
						break;                                /*Successfully removed*/
					}
					else
					{
						MsgQ->Append((void *)myMsg);          /*otherwise append it back*/
						if(CounterBreak==5)
						{
							DEBUG('q',"BREAKING FOR DUPLICATE ACK \n",msgNum,inPktHdr.from,MID,CID); 
							break;
						}
						CounterBreak++;
					}
				}
			}
			else                                      /*if it is a an actual msg*/
			{
				
							if(inPktHdr.from==0)        /*if its client id 0 and its not current thread client id*/
							{
								if(flag0==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck0[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag0++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we Send1????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck0[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
										//	printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							
							if(inPktHdr.from==1)        /*if its client id 1 and its not current thread client id*/
							{
								if(flag1==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck1[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag1++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we send????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck1[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
										//	printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							
							if(inPktHdr.from==2)        /*if its client id 2 and its not current thread client id*/
							{
								if(flag2==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck2[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag2++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we send????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck2[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
											//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							// && k!=myClientID
							if(inPktHdr.from==3)        /*if its client id 3 and its not current thread client id*/
							{
								if(flag3==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck3[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag3++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we send????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck3[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
											//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							
							if(inPktHdr.from==4)        /*if its client id 3 and its not current thread client id*/
							{
								if(flag4==msgNum)          /*flag0 is has the msgnumber which i am expecting from client 0*/ 
								{
									printf("Recieved MSg %d from %d client\n",msgNum,inPktHdr.from);
									SentMsgAck4[msgNum]=msgNum;                  /*keeping track of sent ack's*/
									flag4++;                                     /*next msg number i am expecting*/
									outPktHdr.to = inPktHdr.from;
									outPktHdr.from=myClientID;
									outMailHdr.to = inMailHdr.from;
									outMailHdr.from = 0;
									
									sprintf(SrvBuffer,"%s;%d","ack",msgNum);   /*shd we have to clear or delete this buffer contents once we send????*/
									//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
									
									outMailHdr.length=strlen(SrvBuffer)+1;
									
									DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
									
									if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
										printf("Kernel fn Send failed!\n");
									}	
								}
								else
								{
									/*its an out of order msg,either my ack is lost or in order msg is lost*/
									for(int m=0;m<5;m++)                   /*if u have already acked and still recieved the msg, then ack is lost so resend ack*/
									{
										if(SentMsgAck4[m]==msgNum)          /*shd we initilaize elements of ack array to -1?????*/
										{
											outPktHdr.to = inPktHdr.from;
											outMailHdr.to = inMailHdr.from;
											outMailHdr.from = 0;
											sprintf(SrvBuffer,"%s%c%d","ack",Delimiter,msgNum);
											outMailHdr.length=strlen(SrvBuffer)+1;
											//printf("Ack Msg sent for Seq Number %d from Client %d\n",msgNum,myClientID);
											DEBUG('q',"Sending \"%s\" to client ID %d and box ID %d\n",SrvBuffer,outPktHdr.to,outMailHdr.to);
											if(!postOffice->Send1(outPktHdr,outMailHdr,SrvBuffer)){
												printf("Kernel fn Send failed!\n");
											}
											break;
										}
									}
									//out of order, drop it
								}
								
							}
							
							
			}
	}
}