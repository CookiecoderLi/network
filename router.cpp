#include"header.h"

int main()
{
	int number = 0;
	bool flag = 0;//��־λ����ʾ�Ƿ�õ�IPv4����0Ϊû�еõ���
	BYTE my_mac[6];
	BYTE its_mac[6];
	ULONG my_ip;
	bool arpflag = 0;

	router_table* rt = new router_table[RT_TABLE_SIZE];
	int rt_length = 0;//·�ɱ�ĳ�ʼ����

	pcap_if_t* alldevs;
	pcap_if_t* d;
	pcap_addr_t* a;

	ULONG targetIP;

	char errbuf[PCAP_ERRBUF_SIZE];

	//��ȡ����������Ϣ
	int num = 0;//��¼�м�������ӿڿ�
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING,
		NULL,
		&alldevs,
		errbuf) == -1)
	{
		cout << "have errors" << endl;
	}

	for (d = alldevs; d != NULL; d = d->next)
	{
		cout << "number" << num + 1 << ":";
		cout << "NAME   : " << d->name << endl;//�������ӿڿ��豸������
		cout << "DESCRIPTION: " << d->description << endl;//��ȡ������ӿڿ��豸��������Ϣ
		num++;


		for (a = d->addresses; a != NULL; a = a->next)
		{
			if (a->addr->sa_family == AF_INET)
			{
				cout << "IP��ַ��";
				printIP((((sockaddr_in*)a->addr)->sin_addr).s_addr);
				cout << "�������룺";
				printIP((((sockaddr_in*)a->netmask)->sin_addr).s_addr);
				cout << "�㲥��ַ��";
				printIP((((sockaddr_in*)a->broadaddr)->sin_addr).s_addr);
				cout << endl << endl;
			}
		}

	}
	cout << "����" << num << "������ӿڿ�" << endl;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//��������ȡIP
	cout << "�򿪵ڼ�������ӿڿ���" << endl;
	int in;
	cin >> in;
	in--;
	int i = 0;
	for (d = alldevs; d != NULL && i != in; d = d->next)
	{
		i++;
	}
	//��ӡѡ��������IP���������롢�㲥��ַ
	for (a = d->addresses; a != NULL; a = a->next)
	{
		if (a->addr->sa_family == AF_INET)
		{
			cout << "IP��ַ��";
			printIP((((sockaddr_in*)a->addr)->sin_addr).s_addr);
			cout << "�������룺";
			printIP((((sockaddr_in*)a->netmask)->sin_addr).s_addr);
			cout << "�㲥��ַ��";
			printIP((((sockaddr_in*)a->broadaddr)->sin_addr).s_addr);
			cout << endl;

			ULONG NetMask, DesNet, NextHop;
			DesNet = (((sockaddr_in*)a->addr)->sin_addr).s_addr;
			NetMask = (((sockaddr_in*)a->netmask)->sin_addr).s_addr;
			DesNet = DesNet & NetMask;
			NextHop = 0;
			router_table temp;
			temp.netmask = NetMask;
			temp.desnet = DesNet;
			temp.nexthop = NextHop;
			additem(rt, rt_length, temp);//������Ϣ��ΪĬ��·��
		}
	}



	char errbuf1[PCAP_ERRBUF_SIZE];
	pcap_t* p;//��¼����pcap_open()�ķ���ֵ���������

	p = pcap_open(d->name, 1500, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf1);//������ӿ�

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//���ӻ�ɾ��·�ɱ���

	ULONG NetMask, DesNet, NextHop;
	char* netmask = new char[20];
	char* desnet = new char[20];
	char* nexthop = new char[20];
	bool stop = 1;//stop=0ʱ��ֹͣ�޸�·�ɱ�
	cout << "you wanna change route table? y or n" << endl;
	char ch1;
	cin >> ch1;
	if (ch1 == 'n')
	{
		stop = 0;
		cout << "the route table  is:" << endl;
		print_rt(rt, rt_length);
	}
	while (stop)
	{
		cout << "add or delete?" << endl;
		string str;
		cin >> str;

		if (str == "add")
		{
			cout << "������·�ɱ�����˳��Ϊ��Ŀ������ţ��������룬��һ����" << endl;
			cin >> desnet;
			cin >> netmask;
			cin >> nexthop;
			DesNet = inet_addr(desnet);
			NetMask = inet_addr(netmask);
			NextHop = inet_addr(nexthop);

			router_table temp;
			temp.netmask = NetMask;
			temp.desnet = DesNet;
			temp.nexthop = NextHop;

			additem(rt, rt_length, temp);

			char ch;
			cout << "continue?  y or n" << endl;
			print_rt(rt, rt_length);//��ӡ·�ɱ�
			cin >> ch;
			if (ch == 'n')
			{
				stop = 0;
				cout << "the route table  is:" << endl;
				print_rt(rt, rt_length);
				break;
			}

		}
		else if (str == "delete")
		{
			int index;
			cout << "������Ҫɾ���ı������������㿪ʼ��" << endl;
			cin >> index;//���±�0��ʼ
			deleteitem(rt, rt_length, index);
			char ch;
			cout << "continue?  y or n" << endl;
			cin >> ch;
			if (ch == 'n')
			{
				stop = 0;
				cout << "the route table  is:" << endl;
				print_rt(rt, rt_length);
				break;
			}

		}

	}




	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//���ˣ�ֻҪARP��IP��
	u_int net_mask;
	char packet_filter[] = "ip or arp";
	struct bpf_program fcode;
	net_mask = ((sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	if (pcap_compile(p, &fcode, packet_filter, 1, net_mask) < 0)
		//ʹ��pcap_compile��������һ�����ݰ����˱��ʽ��packet_filter�����������ɹ��������Ĺ��˳��򽫴洢��fcode�ṹ��
	{
		printf("Unable to compile the packet filter.Check the syntax.\n");
		pcap_freealldevs(alldevs);
		return 0;
	}
	if (pcap_setfilter(p, &fcode) < 0)
		//��֮ǰʹ�� pcap_compile ����õ��Ĺ��˳���Ӧ�õ�һ�� pcap �Ự
	{
		printf("Error setting the filter.\n");
		pcap_freealldevs(alldevs);
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//���Լ�����arp������ȡ������MAC
	BYTE scrMAC[6];
	ULONG scrIP;
	for (i = 0; i < 6; i++)
	{
		scrMAC[i] = 0x66;
	}
	scrIP = inet_addr("112.112.112.112");//����IP


	for (d = alldevs, i = 0; i < in; i++, d = d->next);
	for (a = d->addresses; a != NULL; a = a->next)
	{
		if (a->addr->sa_family == AF_INET)
		{
			targetIP = inet_addr(inet_ntoa(((struct sockaddr_in*)(a->addr))->sin_addr));
			my_ip = targetIP;
		}
	}

	ARPFrame_t ARPFrame;
	for (int i = 0; i < 6; i++)
	{
		ARPFrame.FrameHeader.DesMac[i] = 0xff;
		ARPFrame.FrameHeader.SrcMac[i] = scrMAC[i];
		ARPFrame.SendHa[i] = scrMAC[i];
		ARPFrame.RecvHa[i] = 0;
	}
	ARPFrame.FrameHeader.FrameType = htons(0x0806);
	ARPFrame.HardwareType = htons(0x0001);
	ARPFrame.ProtocolType = htons(0x0800);
	ARPFrame.HLen = 6;
	ARPFrame.PLen = 4;
	ARPFrame.Operation = htons(0x0001);
	ARPFrame.SendIP = scrIP;
	ARPFrame.RecvIP = targetIP;
	int ret_send = pcap_sendpacket(p, (u_char*)&ARPFrame, sizeof(ARPFrame_t));

	//ҪĬ�Ϸ����ɹ�  ��Ȼ�����
	/*if (ret_send)
	{
		cout << "���Լ�����ʧ��" << endl;
	}*/
	//else
	//{
	cout << "���Լ������ɹ�" << endl;


	//�ػ��Լ���MAC
	pcap_pkthdr* pkt_header1 = new pcap_pkthdr[1500];
	const u_char* pkt_data1;
	int res;
	ARPFrame_t* ARPFrame1;

	while (!flag)
	{
		res = pcap_next_ex(p, &pkt_header1, &pkt_data1);
		if ((res == 0))
		{
			continue;
		}
		if (res == 1)
		{
			ARPFrame1 = (ARPFrame_t*)pkt_data1;
			if (ARPFrame1->SendIP == targetIP && ARPFrame1->RecvIP == scrIP)
			{
				cout << "����IP:";
				printIP(ARPFrame1->SendIP);
				cout << endl;

				cout << "����MAC:";
				for (int i = 0; i < 6; i++)
				{
					my_mac[i] = ARPFrame1->SendHa[i];
					cout << hex << (int)my_mac[i];
					if (i != 5)cout << "-";
					else cout << endl;
				}
				flag = 1;

			}

		}

	}
	//}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//��ȡĿ��macΪ����mac��Ŀ��ip�Ǳ���ip��ip���ݱ�

	ULONG nextIP;//·�ɵ���һվ
	flag = 0;

	IPData_t* IPPacket;


	pcap_pkthdr* pkt_header = new pcap_pkthdr[1500];
	const u_char* pkt_data;

	int ARPtablesize = 0;
	ARPFrame_t* ARPFrame3 = new ARPFrame_t[256];

	//�����հ�
	while (1)
	{
		//���ݰ��Ļ�ȡ
		int ret_pcap_next_ex;
		ret_pcap_next_ex = pcap_next_ex(p, &pkt_header, &pkt_data);//�ڴ򿪵�����ӿڿ��ϻ�ȡ�������ݰ�
		if (ret_pcap_next_ex)
		{
			//cout << "���ݰ��ĳ���len=" << pkt_header->len << endl;
			WORD RecvChecksum;
			WORD FrameType;

			IPPacket = (IPData_t*)pkt_data;

			ULONG Len = pkt_header->len + sizeof(FrameHeader_t);//���ݰ���С����֡���ݲ��ֳ��Ⱥ�֡�ײ�����
			u_char* sendAllPacket = new u_char[Len];
			for (i = 0; i < Len; i++)
			{
				sendAllPacket[i] = pkt_data[i];
			}

			RecvChecksum = IPPacket->IPHeader.Checksum;
			IPPacket->IPHeader.Checksum = 0;
			FrameType = IPPacket->FrameHeader.FrameType;
			bool desmac_equal = 1;//Ŀ��mac��ַ�뱾��mac��ַ�Ƿ���ͬ����ͬΪ1��
			for (int i = 0; i < 6; i++)
			{
				if (my_mac[i] != IPPacket->FrameHeader.DesMac[i])
				{
					desmac_equal = 0;
				}
			}
			bool desIP_equal = 0;//Ŀ��IP�뱾��IP�Ƿ���ͬ������ͬΪ1��
			if (IPPacket->IPHeader.DstIP != my_ip)
			{
				desIP_equal = 1;
				targetIP = IPPacket->IPHeader.DstIP;
			}
			bool Is_ipv4 = 0;
			if (FrameType == 0x0008)
			{
				Is_ipv4 = 1;
			}


			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (Is_ipv4 && desmac_equal && desIP_equal)//����Ŀ��IP���Ǳ���IP��Ŀ��MACΪ����MAC��IPv4�� 
			{
				cout << "\n��Ϊ�ð�����ת��" << endl;

				int version = (IPPacket->IPHeader.Ver_HLen & 0xf0) >> 4;
				int headlen = (IPPacket->IPHeader.Ver_HLen & 0x0f);
				int tos = IPPacket->IPHeader.TOS;//��������

				int totallen = ntohs(IPPacket->IPHeader.TotalLen);//���ݰ��ܳ���

				int id = ntohs(IPPacket->IPHeader.ID);//��ʶ
				//	int flag_segment = (ntohs(IPPacket->IPHeader.Flag_segment) & 0xe000) >> 13;
				//	int offset = (ntohs(IPPacket->IPHeader.Flag_segment) & 0x1fff);
				int ttl = IPPacket->IPHeader.TTL;
				int protocol = IPPacket->IPHeader.Protocol;



				cout << "version=" << version << endl;
				cout << "headlen=" << headlen << endl;
				cout << "tos=" << dec << tos << endl;
				cout << "totallen=" << dec << totallen << endl;
				cout << "id=" << "0x" << id << endl;
				cout << "ttl=" << dec << ttl << endl;
				cout << "protocol=" << dec << protocol << endl;

				cout << "���ݰ�Դ��ַ��";
				printIP(IPPacket->IPHeader.SrcIP);
				cout << endl;
				cout << "���ݰ�Ŀ�ĵ�ַ��";
				printIP(IPPacket->IPHeader.DstIP);
				cout << endl;

				//ѡ·Ͷ��
				nextIP = search(rt, rt_length, IPPacket->IPHeader.DstIP);
				cout << "·�ɱ���Ϊ��" << rt_length << endl;
				cout << "nextIP:";
				printIP(nextIP);
				cout << endl;
				if (nextIP == 0)
				{
					nextIP = IPPacket->IPHeader.DstIP;
				}
				else if (nextIP == 0xffffffff)
				{
					cout << "�����������ð�" << endl;
				}

				flag = 1;

				//��arp��
				for (int i = 0; i < ARPtablesize; i++) {
					if (ARPFrame3[i].SendIP == nextIP)
					{
						arpflag = 1;
						for (int j = 0; j < 6; j++) {
							its_mac[j] = ARPFrame3[i].SendHa[j];
						}
						
					}
				}

				//���arp��û����Ҫ��ȡmac
				if (arpflag == 0) {

					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
									//��nextIP��arp����ȡMAC��ַ
					for (i = 0; i < 6; i++)
					{
						scrMAC[i] = my_mac[i];
					}
					scrIP = my_ip;


					targetIP = nextIP;

					//��װARP��
					for (int i = 0; i < 6; i++)
					{
						ARPFrame.FrameHeader.DesMac[i] = 0xff;
						ARPFrame.FrameHeader.SrcMac[i] = scrMAC[i];
						ARPFrame.SendHa[i] = scrMAC[i];
						ARPFrame.RecvHa[i] = 0;
					}
					ARPFrame.FrameHeader.FrameType = htons(0x0806);
					ARPFrame.HardwareType = htons(0x0001);
					ARPFrame.ProtocolType = htons(0x0800);
					ARPFrame.HLen = 6;
					ARPFrame.PLen = 4;
					ARPFrame.Operation = htons(0x0001);
					ARPFrame.SendIP = scrIP;
					cout << "sendIP:";
					printIP(ARPFrame.SendIP);
					cout << endl;
					ARPFrame.RecvIP = targetIP;
					cout << "recvIP:";
					printIP(ARPFrame.RecvIP);
					cout << endl;
					int send_ret = pcap_sendpacket(p, (u_char*)&ARPFrame, sizeof(ARPFrame_t));


					/*if (send_ret)
					{
						cout << "����ʧ��" << endl;
					}*/
					//else
					//{
					cout << "�����ɹ�" << endl;



					//�ػ�����MAC

					pcap_pkthdr* pkt_header2 = new pcap_pkthdr[1500];
					const u_char* pkt_data2;

					int res;
					ARPFrame_t* ARPFrame2;

					int flag1 = 0;
					while (!flag1)
					{
						res = pcap_next_ex(p, &pkt_header2, &pkt_data2);



						if ((res == 0))
						{
							continue;

						}
						if (res == 1)
						{
							ARPFrame2 = (ARPFrame_t*)pkt_data2;

							//�����arp����
							for (int i = 0; i < ARPtablesize; i++)
							{
								if (ARPFrame3[i].SendIP == ARPFrame2->SendIP && ARPFrame3[i].SendHa == ARPFrame2->SendHa)
								{
									cout << "ARP������";
									break;
								}
								ARPFrame3[ARPtablesize] = *ARPFrame2;

							}

							if (ARPFrame2->SendIP == nextIP && ARPFrame2->RecvIP == my_ip)
							{
								cout << "NextIP��MAC��ַ:";
								for (int i = 0; i < 6; i++)
								{
									its_mac[i] = ARPFrame2->FrameHeader.SrcMac[i];
									cout << hex << (int)its_mac[i];
									if (i != 5)cout << "-";
									else cout << endl;
								}
								flag1 = 1;
								cout << "NextIP��IP:";
								printIP(ARPFrame2->SendIP);
								cout << endl;
							}
						}

					}

					//}

				}

				arpflag = 0;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//��ĿǰΪֹ֪���ˣ��Լ���IP:my_IP;�Լ���MAC��my_mac[6]��Ҫת����Ŀ��IP:targetIP;Ҫת����Ŀ��MAC:its_mac[6]��������һ�����ͱ�����
				//ת����
				IPData_t* TempIP;
				TempIP = (IPData_t*)sendAllPacket;
				//TempIP->IPHeader.TTL -= 1;//TTL-1 ֻҪ��TTL�ͳ���
				for (int t = 0; t < 6; t++)
				{
					TempIP->FrameHeader.DesMac[t] = its_mac[t];//Ŀ��mac��ַ��Ϊ��һ������ip��ַ��Ӧ��mac��ַ���������䡣
					TempIP->FrameHeader.SrcMac[t] = my_mac[t];
				}
				if (!pcap_sendpacket(p, sendAllPacket, Len))
				{
					cout << "ת���ɹ�" << endl;
					IPData_t* t;
					t = (IPData_t*)sendAllPacket;
					cout << "ԴIP��ַ��";
					printIP(t->IPHeader.SrcIP);
					cout << "\t";

					cout << "Ŀ��IP��ַ��";
					printIP(t->IPHeader.DstIP);
					cout << endl;

					cout << "Ŀ��mac��";
					for (int i = 0; i < 6; i++)
					{
						cout << hex << (int)t->FrameHeader.DesMac[i];
						if (i != 5)cout << "-";
					}
					cout << "\t";
					cout << "Դmac��";
					for (i = 0; i < 6; i++)
					{
						cout << hex << (int)t->FrameHeader.SrcMac[i];
						if (i != 5)cout << "-";
					}
					cout << endl;

				}


			}
		}

	}
	cout << "End" << endl;
	pcap_freealldevs(alldevs);//�ͷ��豸�б�
	return 0;

}