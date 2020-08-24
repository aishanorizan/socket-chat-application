#pragma comment(lib,"ws2_32.lib")
#include<Winsock2.h>
#include<iostream>
#include<string>
#include<ctime>
#include<conio.h>
#include<iostream>
#include<fstream>
using namespace std;

enum Packet
{
	P_ChatMessage, P_Test
};
SOCKET Connections[100];
int loginfailed_check[100][100];
int loginfailed_count = 0;
string logined_user[100];
string stored_name[100];
string stored_msg[100];
int msg_count = 0;
int login_status[100];
int Num_Connection = 0;



bool SendInt(int ID, int _int) //testing socket issues
{
	int RetnCheck = send(Connections[ID], (char*)&_int, sizeof(int), NULL); //send int: _int
	if (RetnCheck == SOCKET_ERROR) //If int failed to send 
		return false; // connecion failure
		return true; // successful connection
}

bool RevInt(int ID, int & _int) //testing socket issues
{
	int RetnCheck = recv(Connections[ID], (char*)&_int, sizeof(int), NULL); //receive paramete - an integer
	if (RetnCheck == SOCKET_ERROR) //If the integer cannot be received
		return false; // connecion failure
		return true;// successful connection
}

bool SendPacketType(int ID, Packet _packettype) //testing socket issues
{
	int RetnCheck = send(Connections[ID], (char*)&_packettype, sizeof(Packet), NULL); //Send packet: _packettype
	if (RetnCheck == SOCKET_ERROR) //If the "packettype" is failed to sent 
		return false; //connection failure
		return true; //successful connection
}

bool RevPacketType(int ID, Packet & _packettype) //testing socket issues
{
	int RetnCheck = recv(Connections[ID], (char*)&_packettype, sizeof(Packet), NULL); //receive "packettype" 
	if (RetnCheck == SOCKET_ERROR) //If the "packettype" cannot be received
		return false; // connection failure
		return true; // successful connection 
}

bool SendString(int ID, std::string & _string)
{
	if (!SendPacketType(ID, P_ChatMessage)) //If send packet type(string value-chat messages) Fails...
		return false; //connection failure
	int bufferlength = _string.size(); //Find string buffer length
	if (!SendInt(ID, bufferlength)) //If length of string buffer fails to send...
		return false;//connection failure
	int RetnCheck = send(Connections[ID], _string.c_str(), bufferlength, NULL); //Send string buffer
	if (RetnCheck == SOCKET_ERROR) //If failed to send string buffer
		return false; //connection failure
	
	return true; // every thing is sent successfully, hence connection is successful
}

bool RevString(int ID, std::string & _string)
{
	int bufferlength; //Stores length of the message
	if (!RevInt(ID, bufferlength)) //Get length of buffer and store in "bufferlength"
		return false; //If get int fails, connection failure

	char * buffer = new char[bufferlength + 1]; //Allocate buffer
	buffer[bufferlength] = '\0'; //Set last character of buffer to null terminator (one of the character & strings rule)
	
	int RetnCheck = recv(Connections[ID], buffer, bufferlength, NULL); //receive message and store the message in buffer array, set RetnCheck to be the value recv returns to see if there is an issue with the connection
	_string = buffer; //set string to receive buffer message
	delete[] buffer; //Deallocate buffer memory (clean up buffer to prevent memory leak)
	
	if (RetnCheck == SOCKET_ERROR) //If connection is lost while getting message
		return false; //Connection has issues
		
	return true;// String is received , hence successful connection
}

bool ProcessPacket(int ID, Packet _packettype)
{
	switch (_packettype)
	{
		case P_ChatMessage: //Packet Type represents : the chat message
			{
			std::string Message; //stringMessage to store received message
			time_t rawtime;
			 struct tm * timeinfo;
			 char buffer[80];

			time (&rawtime);
			timeinfo = localtime(&rawtime);

			strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
			string current_time(buffer);
			
			if (!RevString(ID, Message)) //Get the chat message. Store in variable "Message"
				return false; //Failed to receive message. Connection failure
			
			int Messagelength = Message.size();
			
			int name_i = 0;
			int psd_i = 0;
			int msg_i = 0;
			int msg_j = 0;
			int error_ = 0;
			std::string name;
			std::string password;
			string login_name;
			string msg;
			fstream file ("clientName_list.txt");
			fstream file2 ("clientPassword_list.txt");
			fstream file3 ("clientLogin_list.txt");

			if (Message.substr(0, 7) == "reguser") // register user command
			{
				std::cout << current_time << " < Notice > Registration in process.." << std::endl;
				
				for( int i = 0; i < Messagelength; i++ )
				{				
					if(Message.substr(i, 2) == "-u") // username parameter
					{
						name_i = i+3;						
					}
					if(Message.substr(i, 2) == "-p") // password parameter
					{
						psd_i = i+3;						
					}
				}

				if( name_i == 0 || psd_i == 0 ) //command error checking
				{
					cout << current_time << " < Error > Incomplete command!" << endl;
					std::string error = "\nIncomplete command! \nPlease try again with : reguser -u username -p password";
					SendString(ID, error);
					break;
				}				

				name = Message.substr(name_i, psd_i-4-name_i);
				password = Message.substr(psd_i, Messagelength-psd_i+1);

				cout << current_time << " Username : " << name << "\tPassword : " << password << endl;
				
				if(file.is_open()) // open clientName_list.txt
				{
					file.seekp(0, ios::end);
					file.seekg(0);

					if(file2.is_open()) // open clienPassword_list.txt
					{					
					file2.seekp(0, ios::end);
					string line;

					while(!file.eof()) // open file
					{
						getline(file, line);
						
						if( line == name) // check name
						{
							error_ = 1;						
							std::string error = "\nNew user registration failed! This username is taken.";
							SendString(ID, error);
						}
					}
					cout << current_time << " < Notice > Error check - " << error_ << endl;
					if( error_ == 1 )
					{
						cout << current_time << " < Error > This username already exist. Registration failed!" << endl;
						file.close();
						file2.close();
						break;
					}
					file.seekp(0, ios::end);
					file.seekg(0);
					file2.seekp(0, ios::end);
				
					file << name << endl;
					file2 << password << endl;
					cout << current_time << "Notice : Registration successful!" << endl;

					login_status[ID] = 2;
					std::string error = "Registration successful! You may login now";
					SendString(ID, error);
					
					file2.close();
					}
					file.close(); // close files
				}				
			}
			else if(Message.substr(0, 5) == "login") // login command
			{
				string line;
				string line2;
				error_ = 1;
				
				if( login_status[ID] == 1 )
				{
					error_ = 3;
					goto error_check;
				}
				std::cout << current_time << " Notice : Login in process" << std::endl;
				
				for( int i = 0; i < Messagelength; i++ )
				{			
					if(Message.substr(i, 2) == "-u") // username parameter
					{
						name_i = i+3;						
					}
					if(Message.substr(i, 2) == "-p") // password parameter
					{
						psd_i = i+3;						
					}
				}

				if( name_i == 0 || psd_i == 0 ) // Command error checking
				{
					cout << current_time << " !WARNING! Incomplete command!" << endl;
					std::string error = "\nIncomplete command!\nPlease try again with : login -u username -p password\n";
					SendString(ID, error);
					break;
				}
				
				name = Message.substr(name_i, psd_i-4-name_i);
				password = Message.substr(psd_i, Messagelength-psd_i+1);

				cout << current_time << " Username : " << name << "\tPassword : " << password << endl;

				for (int i = 0; i <= Num_Connection; i++) //Send the message out to each user
				{
					if (login_status[i] == 1 && logined_user[i] == name ) //If connection is the message sender...
					{
							std::string error = "Login failed! This client is currently logged in to the server!";
							SendString(ID, error);

							error_ = 2;							
							break;
					}									
				}		
					
				if( error_ == 2 )
				{
					goto error_check;
				}

				if( file.is_open() ) // open file
				{
					file.seekp(0, ios::end);
					file.seekg(0);

					if( file2.is_open() )
					{
						file2.seekp(0, ios::end);
						file2.seekg(0);

						while((!file.eof()) && (!file2.eof()) ) // Check username, loop to read through one by one
						{
							getline(file, line);
							getline(file2, line2);

							if( line == name && line2 == password) // If user and password matches correctly
							{
									error_ = 0;
						
									std::string msg_1 = ".       ";
									SendString(ID, msg_1);

									logined_user[ID] = name;
									login_status[ID] = 1;
									loginfailed_check[ID][loginfailed_count] = 0;
									loginfailed_count += 1;

									int total_msg = msg_count;

									for(int j = 1; j < 100 ; j++) // check this client got message or not
									{
										if( name == stored_name[j] ) // check username
										{
											string new_msg;												
											new_msg = stored_msg[j];

											cout << current_time << " Notice : Sending stored private/group message to Client < " << logined_user[ID] << " > ..." << endl;
											SendString(ID, new_msg);

											stored_msg[j] = " ";
											stored_name[j] = " ";
											msg_count -= 1; // check message and reset to default value													
										}
									} // end for																	
								}						
						}
						
					error_check:					
						cout << current_time << " Notice : Error check - " << error_ << endl;

					if( error_ == 0 )
					{
						cout << current_time << " Notice : Client < " << name << " > has login to the server!" << endl;
					}
					else if( error_ == 1 )
					{
						login_status[ID] = 0;
						cout << current_time << " Error! : Login failed! Wrong username or password!" << endl;

						for (int i = 0; i < Num_Connection; i++) //Send the message out to each user
						{
							if (i == ID) //If connection is the user who sent the message...
							{
								std::string error = "Login failed! Wrong username or password!";
								SendString(i, error);
								break;
							}									
						}
						loginfailed_check[ID][loginfailed_count] = 1;
						loginfailed_count += 1;
						for( int i = 0; i <= loginfailed_count; i++)
						{
							if(loginfailed_check[ID][i-2] == 1 && loginfailed_check[ID][i-1] == 1 && loginfailed_check[ID][i] == 1) // if client login fails continuously more than 3 times
							{
								cout << current_time << " !WARNING! Client ID [ " << ID << " ] has been blocked for 60 seconds!" << endl;

								for (int i = 0; i < Num_Connection; i++) //Next we need to send the message out to each user
								{
									if (i == ID) //If connection is the user who sent the message...
									{
										std::string error = "!WARNING! Your account has been blocked for 60 seconds!";
										SendString(i, error);
										break;
									}											
								}
							}
						}
					}
					else if( error_ == 2 ) // error check
					{
						login_status[ID] = 0;
						cout << current_time << " !WARNING! Login failed. This client is currently login to the server!" << endl;
						loginfailed_check[ID][loginfailed_count] = 1;
						loginfailed_count += 1;

						for( int i = 0; i <= loginfailed_count; i++)
						{
							if(loginfailed_check[ID][i-2] == 1 && loginfailed_check[ID][i-1] == 1 && loginfailed_check[ID][i] == 1)
							{
								cout << current_time << " !WARNING! Client < " << ID << " > has been blocked for 60 seconds!" << endl;							
								for (int i = 0; i < Num_Connection; i++) //Next we need to send the message out to each user
								{
									if (i == ID) //If connection is the user who sent the message...
									{
										std::string error = "!WARNING! Your account is blocked for 60 seconds!";
										SendString(i, error);
										break;
									} // end if											
								} // end for loop
							} // end if						
				     	} // end for
					} // end else if
					else if ( error_ == 3 ) // error check
					{
						login_status[ID] = 1;
						cout << current_time << " Error! : Client is currently logined to the server!" << endl;
						for (int i = 0; i < Num_Connection; i++) //Next we need to send the message out to each user
						{
							if (i == ID) //If connection is the user who sent the message...
							{
								std::string error = "You are currently logined to the server! Please log out first!";
								SendString(i, error);
								break;
							}									
						}
					} // end if error 3
					file2.close();
				} // close file2
				file.close();
			} // close file				
			} // end case "login"
			else if(Message.substr(0, 3) == "bye") // logout command
			{
				if ( login_status[ID] == 1 )
				{
					cout << current_time << " Notice : Client [ " << logined_user[ID] << " ] has logout from the server!" << endl;
					login_status[ID] = 0;
					logined_user[ID] = " ";
					cout << current_time << " check login status : " << login_status[ID] << endl;										
							std::string error = "You have been logout from the server! Bye!";
							SendString(ID, error);
				}
				else
				{
					cout << current_time << " Notice : Client [ " << ID << " ] haven't login to the server!" << endl;
					for (int i = 0; i < Num_Connection; i++) //Then, send the message out to each user
					{
						if (i == ID) //If connection is the message sender
						{
							std::string error = "You haven't login to the server! Try login now!";
							SendString(i, error);
							break;
						}									
					}
				}
			}
			else if(Message.substr(0, 3) == "who") // who command
			{
				for( int i = 0; i < Messagelength; i++ )
					{					
						if(Message.substr(i, 6) == "-login") // check who login
						{
							cout << current_time << " Notice : Client [ " << logined_user[ID] << " ] requesting for check all logged in username ..." << endl;
							cout << current_time << " Notice : Sending all logged in username to client [ " << logined_user[ID] << " ] " << endl;
							
							for( int i = 0; i < Num_Connection; i++ )
							{
								if (  login_status[i] == 1 )
								{
									string login_name;
									login_name = logined_user[i];																	
									SendString(ID, login_name);
								}																
							}							
						}
						else if(Message.substr(i, 4) == "-all") // check registered users
						{
							cout << current_time << " Notice : Client [ " << logined_user[ID] << " ] requesting for check all registered in username ..." << endl;
							cout << current_time << " Notice : Sending all registered in username to client [ " << logined_user[ID] << " ] " << endl;
							if(file.is_open())
							{
								file.seekg(0);
								string line;
								while(!file.eof())
								{
									getline(file, line);
									string all_name;
									all_name = line;																		
									SendString(ID, all_name);
								}
								file.close();
							}
						}						
					}
			}
			else if(Message.substr(0, 3) == "msg") // messaging command
			{
				int c_count[100];
				int c_num = 0;

				if( login_status[ID] != 1 )
				{
					std::string error = "To send messages, please login first!";
					SendString(ID, error);
								
					break;
				}
				
				for( int i = 0; i < Messagelength; i++ )
				{					
						if(Message.substr(i, 2) == "-b") // Broadcast message
						{
							msg_i = i+3;
							if( msg_i == 0 || Messagelength < 8 )
							{
								cout << current_time << " !WARNING! Incomplete command!" << endl;
								std::string error = "Incomplete command! \nPlease try again with : msg -b message\n";
								SendString(ID, error);
								break;
							}

							msg = ("[ Broadcast Message ] : " + Message.substr(msg_i, Messagelength - msg_i + 1) );
				
							for (int i = 0; i < Num_Connection; i++) //Then, send the message out to each user
							{
								if (i == ID) //If connection is the message sender
									continue; //Skip to the next user (prevent message bounce)
								if (  login_status[i] == 0 || login_status[i] == 1 || login_status[i] == 2 )
								{
									SendString(i, msg);
								}
							}
							break;
						}
						else if(Message.substr(i, 2) == "-p") // send private message
						{
							msg_i = i+3;
							if( msg_i == 0 || Messagelength < 8 )
							{
								cout << current_time << " !WARNING! Incomplete command!" << endl;
								std::string error = "!WARNING! Incomplete command! \nPlease try again with : msg -p message -u username\n";
								SendString(ID, error);
								break;
							}

							for( int j = 0; j < Messagelength; j++ )
							{
								if(Message.substr(j, 2) == "-u")
								{
									msg_j = j+3;									
								}
							}

							if(  msg_j == 0 )
							{
								cout << current_time << " < Error > Incomplete command!" << endl;
								std::string error = "!WARNING! Incomplete command! \nPlease try again with : msg -p message -u username";
								SendString(ID, error);
								break;
							}
							msg =  Message.substr(msg_i, msg_j - msg_i - 4);
							name = Message.substr(msg_j, Messagelength - msg_j + 1);

							string private_message = ("[ Private Message from < " + logined_user[ID] + " > ] : " + msg );

							int name_checker1 = 0;
							int login_checker1 = 0;
							int msg_sent1 = 0;

							if( file.is_open())
							{
								string check_name;
								while( !file.eof())
								{
									getline( file, check_name);
									if( check_name == name ) // this client exists in server register list
									{				
										name_checker1 = 1;
									} // end if
								} // end while

										for (int i = 0; i <= Num_Connection; i++) //Then, send the message out to user
										{																						
											if ( login_status[i] == 1 && logined_user[i] == name )
											{
												if( logined_user[i] == name )
												{
													 msg_sent1 = 1;
												}
												cout << current_time << "Sending private message to Client < " << logined_user[i] << " > ..." << endl;						
												SendString(i, private_message);

												string msg1 = (" Delivery Notice : Private message has been sent to " + logined_user[i] );
												SendString(ID, msg1);
											}
											else
											{												
												string stored_privatemsg = ( private_message + " [ Sent from - " + current_time + " ]");
												
												if(  logined_user[i] != name && login_checker1 ==  0 && name_checker1 == 1 ) // client does not login && client exists in server registration list
												{
														if( msg_sent1 == 1 || i != Num_Connection )
															continue;

														login_checker1 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_privatemsg;
														stored_name[msg_count] = name;
															
														cout << current_time << " Notice : Sending private message to Client < " << name << " > ..." << endl;
														cout << current_time << " Notice : Client < " << name << " > does not login. Message will be sended when the client is login!" << endl;

														string msg1 = ("Sending private message to Client < " + name + " > ..." );
														string msg2 = ("Client < " + name + " > does not login. Message will be sended when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
												}												
											} // end else											
										} // end for
								if( file.eof() && name_checker1 == 0 )
								{									
									cout << current_time << " Error ! : Client < " << name << " > does not exists in our server registration list!" << endl;
									string msg1 = ("Client < " + name + " > does not exists in our server registration list!");
									SendString(ID, msg1);
								}
							file.close();
							}
						}
						else if(Message.substr(i, 2) == "-g") // send group message
						{
							msg_i = i+3;

							if( msg_i == 0 || Messagelength < 13 )
							{
								cout << current_time << " Error ! : Incomplete command!" << endl;
								std::string error = "\n !WARNING!Incomplete command! \nPlease try again with : msg -g message -u username1, username2, usernameN";
								SendString(ID, error);
								break;
							}

							for( int j = 0; j < Messagelength; j++ )
							{
								if(Message.substr(j, 2) == "-u")
								{
									msg_j = j+3;									
								}
							
								if(Message.substr(j, 1) == ",")
								{
									c_num++;
									c_count[c_num] = j+2;									
								}
							}

							if(  msg_j == 0 ) 
							{
								cout << current_time << " Error! : Incomplete command!" << endl;
								std::string error = "!WARNING! Incomplete command! Please try again with : msg -g message -u username1, username2, usernameN";
								SendString(ID, error);
								break;
							}							
							msg = Message.substr(msg_i, msg_j - msg_i - 4);

							string group_message = ("[ Group Message from < " + logined_user[ID] + " > ] : " + msg );
							
							if( c_num == 0 ) // send group message to 1 client
							{
								name = Message.substr(msg_j, Messagelength - msg_j + 1);

								int name_checker1 = 0;
								int login_checker1 = 0;
								int msg_sent1 = 0;
								
								if( file.is_open())
								{
									string name_check;
									while( !file.eof())
									{
										getline(file, name_check);
												if( name_check == name ) // this client exists in the server registration list
											{
												name_checker1 = 1;
											} // end if
									} // end while
								
												for (int i = 0; i <= Num_Connection; i++) //Next we need to send the message out to each user
												{																											
													if ( login_status[i] == 1 && logined_user[i] == name )
													{
														if( logined_user[i] == name )
														{
															 msg_sent1 = 1;
														}
														cout << current_time << "\nSending group message to Client < " << logined_user[i] << " > ..." << endl;
														SendString(i, group_message);

														string msg1 = ("\n Notice : Group message has been sent to " + logined_user[i] );
														SendString(ID, msg1);
													}
													else
													{
														string stored_groupmsg = ( group_message + " [ Sent from - " + current_time + " ]");

														if(  logined_user[i] != name && login_checker1 ==  0 && name_checker1 == 1 ) // client does not login && client exists in server registration list
														{
															if( msg_sent1 == 1 || i != Num_Connection )
																continue;

															login_checker1 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name;
															
															cout << current_time << " \nSending group message to Client < " << name << " > ..." << endl;
															cout << current_time << "\n Notice : Client < " << name << " > does not login. Message will be sended when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name + " > ..." );
															string msg2 = ("Client < " + name + " > does not login. Message will be sended when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}												
													}
												} // end for										
									if( file.eof() && name_checker1 == 0 )
									{
										cout << current_time << "  Error! : Client < " << name << " > does not exists in our server!" << endl;
										string msg1 = (" Notice : Client < " + name + " > does not exists in our server!");
										SendString(ID, msg1);
									}
								file.close();
								} // end if open file
							}
							else if( c_num == 1 )
							{
								string name2;

								int login_checker1 = 0;
								int login_checker2 = 0;

								name = Message.substr(msg_j, c_count[1] - msg_j - 2);
								name2 = Message.substr(c_count[1], Messagelength - c_count[1] + 1);

								int name_checker1 = 0;
								int name_checker2 = 0;
								int msg_sent1 = 0;
								int msg_sent2 = 0;

								if( file.is_open())
								{
									string name_check;
									while( !file.eof())
									{
										getline( file, name_check);
										if( name_check == name || name_check == name2 ) // client name exists in server registration list
										{
											if( name_check == name )
											{
												name_checker1 = 1;
											}
											if( name_check == name2 )
											{
												name_checker2 = 1;
											}
										} // end if  name_check == name || name_check == name2
									} // end while check name
										
											for (int i = 0; i <= Num_Connection; i++) //Next we need to send the message out to each user
											{																							
												if ( login_status[i] == 1 && (logined_user[i] == name || logined_user[i] == name2 ) )
												{
													if( logined_user[i] == name )
													{
														 msg_sent1 = 1;
													}
													if( logined_user[i] == name2 )
													{
														 msg_sent2 = 1;
													}													
													cout << current_time << " Notice : Sending group message to Client < " << logined_user[i] << " > ..." << endl;
													SendString(i, group_message);

													string msg1 = (" Delivery Notice : Group message has been sent to " + logined_user[i] );
													SendString(ID, msg1);													
												}
												else
												{
													string stored_groupmsg = ( group_message + " [ Sent from - " + current_time + " ]");
																								
													if(  logined_user[i] != name && login_checker1 ==  0 && name_checker1 == 1 ) // client does not login && client exists in server registration list
													{
														if( msg_sent1 == 1 || i != Num_Connection )
															goto if_2;

														login_checker1 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_groupmsg;
														stored_name[msg_count] = name;
														
														cout << current_time << " Notice : Sending group message to Client < " << name << " > ..." << endl;
														cout << current_time << " Notice : Client < " << name << " > does not login. Message will only be sent when the client is login!" << endl;

														string msg1 = ("Sending group message to Client < " + name + " > ..." );
														string msg2 = (" Notice :Client < " + name + " > does not login. Message will only be sent when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
													}
													if_2:
													if(  logined_user[i] != name2 && login_checker2 ==  0 && name_checker2 == 1 ) // client2 does not login && client exists in server registration list
													{
														if( msg_sent2 == 1 || i != Num_Connection)
															continue;

														login_checker2 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_groupmsg;
														stored_name[msg_count] = name2;
														
														cout << current_time << " Notice : Sending group message to Client < " << name2 << " > ..." << endl;
														cout << current_time << " Notice : Client < " << name2 << " > does not login. Message will be sended when the client is login!" << endl;

														string msg1 = ("Sending group message to Client < " + name2 + " > ..." );
														string msg2 = (" Notice : Client < " + name2 + " > does not login. Message will only be when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
													}																										
												} // else if
											} // end for
										
									if( file.eof() && name_checker1 == 0 ) // client name does not exists in server registration list
									{
										cout << current_time << " Error! : Client < " << name << " > does not exists in our server!" << endl;
											string msg1 = ("Notice : Client < " + name + " > does not exists in our server!");
											SendString(ID, msg1);											
									}
									if( file.eof() && name_checker2 == 0 ) // client name does not exists in server registration list
									{
										cout << current_time << " Error! : Client < " << name2 << " > does not exists in our server!" << endl;
											string msg1 = ("Notice : Client < " + name2 + " > does not exists in our server!");
											SendString(ID, msg1);								
									}
									file.close();
								} // end if open file
							}
							else if( c_num == 2 ) // send group message to 3 clients
							{
								string name2;
								string name3;

								name = Message.substr(msg_j, c_count[1] - msg_j - 2);
								name2 = Message.substr(c_count[1], c_count[2] - c_count[1] - 2);
								name3 = Message.substr(c_count[2], Messagelength - c_count[2] + 1);

								int name_checker1 = 0;
								int name_checker2 = 0;
								int name_checker3 = 0;

								int login_checker1 = 0;
								int login_checker2 = 0;
								int login_checker3 = 0;
								
								int msg_sent1 = 0;
								int msg_sent2 = 0;
								int msg_sent3 = 0;							

								if( file.is_open())
								{
									string name_check;
									while( !file.eof())
									{
										getline( file, name_check);										
										if( name_check == name || name_check == name2 || name_check == name3 )
										{
											if( name_check == name )
											{
												name_checker1 = 1;
											}
											if( name_check == name2 )
											{
												name_checker2 = 1;
											}
											if( name_check == name3 )
											{
												name_checker3 = 1;
											}

										} // end if
									} // end while
								
												for (int i = 0; i <= Num_Connection; i++) //Next we need to send the message out to each user
												{																							
													if ( login_status[i] == 1 && (logined_user[i] == name || logined_user[i] == name2 || logined_user[i] == name3 ) ) // client is logged in and username is correct
													{
														if( logined_user[i] == name )
														{
															 msg_sent1 = 1;
														}
														if( logined_user[i] == name2 )
														{
															 msg_sent2 = 1;
														}
														if( logined_user[i] == name3 )
														{
															 msg_sent3 = 1;
														}
														cout << current_time << " Notice : Sending group message to Client < " << logined_user[i] << " > ..." << endl;
														SendString(i, group_message);

														string msg1 = (" Delivery Notice : Group message has been sent to " + logined_user[i] );
														SendString(ID, msg1);
													}
													else
													{
														string stored_groupmsg = ( group_message + " [ Sent from - " + current_time + " ]");

														if(  logined_user[i] != name && login_checker1 ==  0 && name_checker1 == 1 ) // client does not login && client exists in server registration list
														{
															if( msg_sent1 == 1 || i != Num_Connection)
																goto if1_2;

															login_checker1 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name;
														
															cout << current_time << " Notice : Sending group message to Client < " << name << " > ..." << endl;
															cout << current_time << " Notice : Client < " << name << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name + " > ..." );
															string msg2 = ("Notice : Client < " + name + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
														if1_2:
														if(  logined_user[i] != name2 && login_checker2 ==  0 && name_checker2 == 1 ) // client2 does not login && client exists in server registration list
														{
															if( msg_sent2 == 1 || i != Num_Connection )
																goto if1_3;

															login_checker2 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name2;
															
															cout << current_time << "Notice : Sending group message to Client < " << name2 << " > ..." << endl;
															cout << current_time << "Notice : Client < " << name2 << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name2 + " > ..." );
															string msg2 = ("Notice : Client < " + name2 + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
														if1_3:
														if(  logined_user[i] != name3 && login_checker3 ==  0 && name_checker3 == 1 ) // client2 does not login && client exists in server registration list
														{
															if( msg_sent3 == 1 || i != Num_Connection )
																continue;

															login_checker3 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name3;
															
															cout << current_time << " Notice : Sending group message to Client < " << name3 << " > ..." << endl;
															cout << current_time << " Notice : Client < " << name3 << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name3 + " > ..." );
															string msg2 = ("Notice : Client < " + name3 + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
													}
												} // end for
										
									if( file.eof() && name_checker1 == 0 )
									{
										cout << current_time << " Error! : Client < " << name << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									if( file.eof() && name_checker2 == 0 )
									{
										cout << current_time << " Error! : Client < " << name2 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name2 + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									if( file.eof() && name_checker3 == 0 )
									{
										cout << current_time << " Error! : Client < " << name3 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name3 + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									file.close();
								} // end if
							}
							else if( c_num == 3 )
							{
								string name2;
								string name3;
								string name4;

								name = Message.substr(msg_j, c_count[1] - msg_j - 2);
								name2 = Message.substr(c_count[1], c_count[2] - c_count[1] - 2);
								name3 = Message.substr(c_count[2], c_count[3] - c_count[2] - 2);
								name4 = Message.substr(c_count[3], Messagelength - c_count[3] + 1);

								int name_checker1 = 0;
								int name_checker2 = 0;
								int name_checker3 = 0;
								int name_checker4 = 0;

								int login_checker1 = 0;
								int login_checker2 = 0;
								int login_checker3 = 0;
								int login_checker4 = 0;

								int msg_sent1 = 0;
								int msg_sent2 = 0;
								int msg_sent3 = 0;
								int msg_sent4 = 0;

								if( file.is_open())
								{
									string name_check;
									while( !file.eof())
									{
										getline( file, name_check);
										if( name_check == name || name_check == name2 || name_check == name3 || name_check == name4 )
										{
											if( name_check == name )
											{
												name_checker1 = 1;
											}
											if( name_check == name2 )
											{
												name_checker2 = 1;
											}
											if( name_check == name3 )
											{
												name_checker3 = 1;
											}
											if( name_check == name4 )
											{
												name_checker4 = 1;
											}
										} // end if
									} // end while
								
											for (int i = 0; i <= Num_Connection; i++) //Next we need to send the message out to each user
											{																							
												if ( login_status[i] == 1 && (logined_user[i] == name || logined_user[i] == name2 || logined_user[i] == name3 || logined_user[i] == name4) )
												{
													if( logined_user[i] == name )
													{
														 msg_sent1 = 1;
													}
													if( logined_user[i] == name2 )
													{
														 msg_sent2 = 1;
													}
													if( logined_user[i] == name3 )
													{
														 msg_sent3 = 1;
													}
													if( logined_user[i] == name4 )
													{
														 msg_sent4 = 1;
													}
													cout << current_time << "Sending group message to Client < " << logined_user[i] << " > ..." << endl;
													SendString(i, group_message);

													string msg1 = (" Delivery Notice : Group message has been sent to " + logined_user[i] );
													SendString(ID, msg1);
												}
												else
												{
													string stored_groupmsg = ( group_message + " [ Sent from - " + current_time + " ]");

													if(  logined_user[i] != name && login_checker1 ==  0 && name_checker1 == 1 ) // client does not login && client exists in server registration list
													{
														if( msg_sent1 == 1 || i != Num_Connection )
															goto if2_2;

														login_checker1 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_groupmsg;
														stored_name[msg_count] = name;
														
														cout << current_time << "Sending group message to Client < " << name << " > ..." << endl;
														cout << current_time << " Notice : Client < " << name << " > does not login. Message will only be sent when the client is login!" << endl;

														string msg1 = (" Sending group message to Client < " + name + " > ..." );
														string msg2 = (" Client < " + name + " > does not login. Message will only be sent when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
													}
													if2_2:
													if(  logined_user[i] != name2 && login_checker2 ==  0 && name_checker2 == 1 ) // client2 does not login && client exists in server registration list
													{
														if( msg_sent2 == 1 || i != Num_Connection )
															goto if2_3;

														login_checker2 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_groupmsg;
														stored_name[msg_count] = name2;
																												cout << current_time << "Sending group message to Client < " << name2 << " > ..." << endl;
														cout << current_time << " Notice : Client < " << name2 << " > does not login. Message will only be sent when the client is login!" << endl;

														string msg1 = ("Sending group message to Client < " + name2 + " > ..." );
														string msg2 = ("Notice : Client < " + name2 + " > does not login. Message will only be sent when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
													}
													if2_3:
													if(  logined_user[i] != name3 && login_checker3 ==  0 && name_checker3 == 1 ) // client2 does not login && client exists in server registration list
													{
														if( msg_sent3 == 1 || i != Num_Connection  )
															goto if2_4;

														login_checker3 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_groupmsg;
														stored_name[msg_count] = name3;
														
														cout << current_time << "Sending group message to Client < " << name3 << " > ..." << endl;
														cout << current_time << "Notice : Client < " << name3 << " > does not login. Message will only be sent when the client is login!" << endl;

														string msg1 = ("Sending group message to Client < " + name3 + " > ..." );
														string msg2 = ("Notice : Client < " + name3 + " > does not login. Message will only be sent when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
													}
													if2_4:
													if(  logined_user[i] != name4 && login_checker4 ==  0 && name_checker4 == 1 ) // client2 does not login && client exists in server registration list
													{
														if( msg_sent4 == 1 || i != Num_Connection )
															continue;

														login_checker4 = 1;
														msg_count += 1;
														stored_msg[msg_count] = stored_groupmsg;
														stored_name[msg_count] = name4;
														
														cout << current_time << "Sending group message to Client < " << name4 << " > ..." << endl;
														cout << current_time << " Notice : Client < " << name4 << " > does not login. Message will only be sent when the client is login!" << endl;

														string msg1 = ("Sending group message to Client < " + name4 + " > ..." );
														string msg2 = ("Notice : Client < " + name4 + " > does not login. Message will only be sent when the client is login!" );
														SendString(ID, msg1);
														SendString(ID, msg2);
													}
												}
											} // end for

									if( file.eof() && name_checker1 == 0 )
									{
										cout << current_time << " Error! : Client < " << name << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									if( file.eof() && name_checker2 == 0 )
									{
										cout << current_time << "  Error! : Client < " << name2 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name2 + " > does not exists in our server!");
										SendString(ID, msg1);								
									}
									if( file.eof() && name_checker3 == 0 )
									{
										cout << current_time << "  Error! : Client < " << name3 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name3 + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									if( file.eof() && name_checker4 == 0 )
									{
										cout << current_time << " Error! : Client < " << name4 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name4 + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									file.close();
								} // end if
							}
							else if( c_num == 4 )
							{
								string name2;
								string name3;
								string name4;
								string name5;

								name = Message.substr(msg_j, c_count[1] - msg_j - 2);
								name2 = Message.substr(c_count[1], c_count[2] - c_count[1] - 2);
								name3 = Message.substr(c_count[2], c_count[3] - c_count[2] - 2);
								name4 = Message.substr(c_count[3], c_count[4] - c_count[3] - 2);
								name5 = Message.substr(c_count[4], Messagelength - c_count[4] + 1);

								int name_checker1 = 0;
								int name_checker2 = 0;
								int name_checker3 = 0;
								int name_checker4 = 0;
								int name_checker5 = 0;

								int login_checker1 = 0;
								int login_checker2 = 0;
								int login_checker3 = 0;
								int login_checker4 = 0;
								int login_checker5 = 0;

								int msg_sent1 = 0;
								int msg_sent2 = 0;
								int msg_sent3 = 0;
								int msg_sent4 = 0;
								int msg_sent5 = 0;

								if( file.is_open())
								{
									string name_check;

									while( !file.eof())
									{
										getline( file, name_check);

										if( name_check == name || name_check == name2 || name_check == name3 || name_check == name4 || name_check == name5 )
										{
											if( name_check == name )
											{
												name_checker1 = 1;
											}
											if( name_check == name2 )
											{
												name_checker2 = 1;
											}
											if( name_check == name3 )
											{
												name_checker3 = 1;
											}
											if( name_check == name4 )
											{
												name_checker4 = 1;
											}
											if( name_check == name5 )
											{
												name_checker5 = 1;
											}
										} // end if
									} // end while

												for (int i = 0; i <= Num_Connection; i++) //Next we need to send the message out to each user
												{																									
												if ( login_status[i] == 1 && (logined_user[i] == name || logined_user[i] == name2 || logined_user[i] == name3 || logined_user[i] == name4 || logined_user[i] == name5) )
													{
														if( logined_user[i] == name )
														{
															 msg_sent1 = 1;
														}
														if( logined_user[i] == name2 )
														{
															 msg_sent2 = 1;
														}
														if( logined_user[i] == name3 )
														{
															 msg_sent3 = 1;
														}
														if( logined_user[i] == name4 )
														{
															 msg_sent4 = 1;
														}
														if( logined_user[i] == name5 )
														{
															 msg_sent5 = 1;
														}

														cout << current_time << " Notice : Sending group message to Client < " << logined_user[i] << " > ..." << endl;
														SendString(i, group_message);

														string msg1 = (" Delivery Notice : Group message has been sent to " + logined_user[i] );
														SendString(ID, msg1);
													}
													else
													{
														string stored_groupmsg = ( group_message + " [ Sent from - " + current_time + " ]");

														if(  logined_user[i] != name && login_checker1 ==  0 && name_checker1 == 1 ) // client does not login && client exists in server registration list
														{
															if( msg_sent1 == 1 || i != Num_Connection )
																goto if3_2;

															login_checker1 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name;
															
															cout << current_time << "Sending group message to Client < " << name << " > ..." << endl;
															cout << current_time << "Notice : Client < " << name << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name + " > ..." );
															string msg2 = ("Notice : Client < " + name + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
														if3_2:
														if(  logined_user[i] != name2 && login_checker2 ==  0 && name_checker2 == 1 ) // client2 does not login && client exists in server registration list
														{
															if( msg_sent2 == 1 || i != Num_Connection )
																goto if3_3;

															login_checker2 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name2;
															
															cout << current_time << " Notice : Sending group message to Client < " << name2 << " > ..." << endl;
															cout << current_time << " Notice : Client < " << name2 << " > does not login. Message will be sended when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name2 + " > ..." );
															string msg2 = ("Notice : Client < " + name2 + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
														if3_3:
														if(  logined_user[i] != name3 && login_checker3 ==  0 && name_checker3 == 1 ) // client2 does not login && client exists in server registration list
														{
															if( msg_sent3 == 1 || i != Num_Connection)
																goto if3_4;

															login_checker3 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name3;
															
															cout << current_time << "Sending group message to Client < " << name3 << " > ..." << endl;
															cout << current_time << " Notice : Client < " << name3 << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name3 + " > ..." );
															string msg2 = ("Notice : Client < " + name3 + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
														if3_4:
														if(  logined_user[i] != name4 && login_checker4 ==  0 && name_checker4 == 1 ) // client2 does not login && client exists in server registration list
														{
															if( msg_sent4 == 1 || i != Num_Connection )
																goto if3_5;

															login_checker4 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name4;
															
															cout << current_time << "Sending group message to Client < " << name4 << " > ..." << endl;
															cout << current_time << "Notice : Client < " << name4 << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name4 + " > ..." );
															string msg2 = ("Notice : Client < " + name4 + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
														if3_5:
														if(  logined_user[i] != name5 && login_checker5 ==  0 && name_checker5 == 1 ) // client2 does not login && client exists in server registration list
														{
															if( msg_sent5 == 1 || i != Num_Connection )
																continue;
															login_checker5 = 1;
															msg_count += 1;
															stored_msg[msg_count] = stored_groupmsg;
															stored_name[msg_count] = name5;
														
															cout << current_time << " Notice : Sending group message to Client < " << name5 << " > ..." << endl;
															cout << current_time << " Notice : Client < " << name5 << " > does not login. Message will only be sent when the client is login!" << endl;

															string msg1 = ("Sending group message to Client < " + name5 + " > ..." );
															string msg2 = ("Notice : Client < " + name5 + " > does not login. Message will only be sent when the client is login!" );
															SendString(ID, msg1);
															SendString(ID, msg2);
														}
													} // end else
												} // end for

									if( file.eof() && name_checker1 == 0 )
									{
										cout << current_time << " Error! : Client < " << name << " > does not exist in server!" << endl;
										string msg1 = ("Notice : Client : " + name + " > does not exists in our server!");
										SendString(ID, msg1);									
									}
									if( file.eof() && name_checker2 == 0 )
									{
										cout << current_time << " Error : Client < " << name2 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name2 + " > does not exists in our server!");
										SendString(ID, msg1);								
									}
									if( file.eof() && name_checker3 == 0 )
									{
										cout << current_time << "  Error : Client < " << name3 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name3 + " > does not exists in our server!");
										SendString(ID, msg1);								
									}
									if( file.eof() && name_checker4 == 0 )
									{
										cout << current_time << " Error! : Client < " << name4 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name4 + " > does not exists in our server");
										SendString(ID, msg1);				
									}
									if( file.eof() && name_checker5 == 0 )
									{
										cout << current_time << " Error! : Client < " << name5 << " > does not exists in our server!" << endl;
										string msg1 = ("Notice : Client < " + name5 + " > does not exists in our server!");
										SendString(ID, msg1);	
									}
									file.close();
								} // end if file is open
							} // end else if						
							else
							{
								cout << current_time << " Error! : The group messaging function cannot support more than 5 users!" << endl;
								string error = "Notice :The group messaging function cannot support more than 5 users!";
								SendString(ID, error);
							}	
						} // else if -g
				} // end for
			} // end else if msg
			std::cout << current_time << "Notice : Processed command / message packet from user ID: " << ID << std::endl;
			break; 
		}// end case msg packet type
		default: //If packet type is not accounted for
		{
			time_t rawtime;
			 struct tm * timeinfo;
			 char buffer[80];

			time (&rawtime);
			timeinfo = localtime(&rawtime);

			strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
			string current_time(buffer);
			
			std::cout << current_time << "Unrecognized packet: " << _packettype << std::endl; //Display that packet was not found
			break;
		}
	} // end switch case
	return true;
} // end bool
void ClientHandlerThread(int ID) 
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
	string current_time(buffer);
			
	Packet PacketType;
	while (true)
	{
		if (!RevPacketType(ID, PacketType)) //Get packet type
			break; //If there is an issue getting the packet type, exit this loop
		if (!ProcessPacket(ID, PacketType)) //Process packet (packet type)
			break; //If there is an issue processing the packet, exit this loop
	}
	std::cout << current_time << " Lost connection to client ID: " << ID << std::endl;
	Num_Connection -= 1;
	login_status[ID] = 0;
	logined_user[ID] = " ";
	closesocket(Connections[ID]);
}

int main()
{ 
	//Winsock Startup
	WSADATA wsaData;
	WORD DllVersion = MAKEWORD(2,1);
	if(WSAStartup(DllVersion,&wsaData)!=0)//If WSAStartup returns anything that are not 0
	{MessageBox(NULL,
	L"Winsock startup failed",
	L"Error",MB_OK | MB_ICONERROR);
		exit(1);	
	}else
	{	
	cout << endl ;
	cout <<"-----------------------------------------------------------------"<< endl;  
	cout <<"\t\t\t  S E R V E R"<< endl; 
	cout <<"-----------------------------------------------------------------\n"<< endl;
		for(int i = 0; i<=100; i++){
		int r = rand() % 100;
		cout<< "\r"<<"\t\tInitiating server system "<< i << "% . . ." <<flush;
			
	}
	
	cout<< "\r"<<"\t     Server is online. Waiting for connections...  " <<flush;
	cout << endl<<endl;
	}
	
	fstream file("clientName_list.txt",std::ios::out | std::ios::in | std::ios::app);
	fstream file2("clientPassword_list.txt",std::ios::out | std::ios::in | std::ios::app);
	
	if (file.is_open())
	{
		file.close();
	}
	if (file2.is_open())
	{
		file2.close();
	}
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
	std::string current_time(buffer);

	SOCKADDR_IN addr; //Address that we will blind out listening socket to
	int addrlen = sizeof(addr); // length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Broadcast in local address
	addr.sin_port = htons(1111); // Port
	addr.sin_family = AF_INET;// IPv4 Socket

	SOCKET sListen = socket(AF_INET , SOCK_STREAM,NULL); // create socket to await for new connections
	bind(sListen,(SOCKADDR*)&addr,sizeof(addr)); //Bind the address to the socket
	listen(sListen,SOMAXCONN); //Places sListen socket in "waiting phase" (for connections from client)
	SOCKET newConnection; // Socket to hold the client's connection
	for(int i =0;i<100; i++)
	{
		newConnection = accept(sListen,(SOCKADDR*)&addr,&addrlen); //Aceept a new connection
		if(newConnection ==0)// if accepting the client connection failed
		{ std::cout << current_time <<" Failed to accept the client's connection." <<std::endl;
		} 
		else // client connection is successful
		{ 
			login_status[i]=3;
			std::cout << current_time <<" A client is requesting for connection ..."<<endl;
			std::cout << current_time <<" A client is connected !" << std::endl;
			Connections[i]= newConnection;
			Num_Connection += 1;
			CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ClientHandlerThread,(LPVOID)(i),NULL,NULL); //Create Thread to handle the client.

			std::string msg_1 = " .  ";

			SendString(i, msg_1);
		}
	}
	system("pause");
	return 0;
}
