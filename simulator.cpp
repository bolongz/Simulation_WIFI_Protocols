
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <algorithm>


std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

const double SPEED = 6.0;
const int ACM = 44;
const int RTS_CTS = 30;
const int SLOT = 9;
const int DIFS = 28;
const int SIFS = 10;
const int CW = 32;

struct packet {
  int pkt_id;
  int scr_node;
  int dst_node;
  int pkt_size;
  int time;
  int new_time;
  int dcf_time;
  int back_off_time;
  int slots = -1; //initial value to be negative 1
  int trans_time;
  bool collision = false; //to show if the
  int repeat = 0; // retransimission time
  int end_time;
  int sequence = -1;
};
struct packet_temp{
  char pkt_id[5];
  char scr_node[5];
  char dst_node[5];
  char pkt_size[5];
  char time[10];
};
void convert_int (char *strLine,struct packet * packet_out) {
  struct packet_temp packet_out_temp;
  sscanf( strLine, "%s %s %s %s %s",packet_out_temp.pkt_id,packet_out_temp.scr_node,packet_out_temp.dst_node,packet_out_temp.pkt_size,packet_out_temp.time);
  packet_out->pkt_id=atoi(packet_out_temp.pkt_id);
  packet_out->scr_node=atoi(packet_out_temp.scr_node);
  packet_out->dst_node=atoi(packet_out_temp.dst_node);
  packet_out->pkt_size=atoi(packet_out_temp.pkt_size);
  packet_out->time=atoi(packet_out_temp.time);
  packet_out->new_time = packet_out->time;
  packet_out->trans_time = int(packet_out->pkt_size / SPEED + 0.5) + 44 + 10;

}

int random_int(int s, int e){
  std::uniform_int_distribution<> random_int(s, e);
  return random_int(gen);
}

void dcf_mac_protocol(struct packet *packet_out,int no){
  // size_t channel = 0; // 0 for free, 1 for busy
  //int channel_end = 0;
  std::vector<packet>  dcf_wait_list;
  std::vector<packet>  back_up;
  std::vector<bool> sent_flag(no,0);
  std::vector<int> vector_index;
  int _max = packet_out[0].new_time;
  for(int i = 0;i < no; i++){
    packet_out[i].sequence = i;
    // std::cout << packet_out[i].new_time << " XXXX " << _max << std::endl;
    while(packet_out[i].new_time <= _max  && i < no){

      if(!sent_flag[i]){
        packet_out[i].sequence = i;
        //std::cout << "HEEEEEEEEEEEEEEEEEE " << std::endl;
        printf("Time: %d, Node %d waiting for DIFS\n",packet_out[i].new_time,packet_out[i].pkt_id);
        packet_out[i].dcf_time = packet_out[i].new_time + DIFS;
        if(packet_out[i].slots == -1){
          if(!packet_out[i].collision){
            packet_out[i].slots = random_int(0, 15);
          }else{
            if(packet_out[i].repeat > 10){
              printf("Time: %d, Node %d sent failed\n",packet_out[i].new_time,packet_out[i].pkt_id);
            }
            packet_out[i].slots = random_int(0, 32 * packet_out[i].repeat);
          }
        }
        packet_out[i].back_off_time = packet_out[i].dcf_time + SLOT * packet_out[i].slots;
        packet_out[i].end_time =packet_out[i].back_off_time + packet_out[i].trans_time;

        dcf_wait_list.push_back(packet_out[i]);
        if(packet_out[i].back_off_time > _max){
          _max = packet_out[i].back_off_time;
        }
      }
      i++;
      if(i == no) {
        i = i -1;
        break;
      }

    }
    //std::cout <<"max " <<  _max << std::endl;
    std::sort(dcf_wait_list.begin(), dcf_wait_list.end(), [](struct packet left, struct packet right) { return (left.back_off_time) < (right.back_off_time);});
    // checking the sending and collision
    int back_up_size = dcf_wait_list.size(); // make backup
    //std::cout << back_up_size <<"  xxx " << std::endl;

    if(dcf_wait_list.size() == 1){
      printf("Time: %d, Node %d finished waiting for DIFS and started waiting for %d slots \n",dcf_wait_list[0].dcf_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].slots);
      printf("Time: %d, Node %d finished waiting and is ready to send the packet \n",dcf_wait_list[0].back_off_time, dcf_wait_list[0].pkt_id);
      printf("Time: %d, Node %d send %d \n",dcf_wait_list[0].end_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].pkt_size);

      sent_flag[dcf_wait_list[0].sequence] = true;
      std::vector<packet>().swap(dcf_wait_list);
      if(no > 1){
        _max = packet_out[1].new_time;
      }
      //std::cout << "MAX 2 " << _max << std::endl;
       i = i -1;
      continue;
    }else{
      bool _collision = false;
      //std::cout << " HERE " << dcf_wait_list.size() << std::endl;
      for(size_t ii = 1; ii < dcf_wait_list.size(); ii++){

        while(dcf_wait_list[ii].back_off_time ==  dcf_wait_list[0].back_off_time&& ii < dcf_wait_list.size()){
          if( ii == 1){
            dcf_wait_list[0].repeat += 1;
            dcf_wait_list[0].collision = true;
            dcf_wait_list[0].slots = -1; //random_int(0, 32 * dcf_wait_list[0].repeat);
            // dcf_wait_list[0].back_off_time = dcfpacket_out[i].dcf_time + slot * packet_out[i].slots;
            // dcf_wait_list[0].end_time = dcf_wait_list[0].back_off_time + dcf_wait_list[0].trans_time;
          }
          dcf_wait_list[ii].repeat += 1;
          dcf_wait_list[ii].collision = true;
          dcf_wait_list[ii].slots = -1; //random_int(0, 32 * dcf_wait_list[i].repeat);
          // dcf_wait_list[i].back_off_time = dcfpacket_out[i].dcf_time + slot * packet_out[i].slots;
          // dcf_wait_list[i].end_time = dcf_wait_list[i].back_off_time + dcf_wait_list[i].trans_time;
          ii++;
          _collision = true; // collision detedte;
        }
        if(!_collision){
          //std::cout << "NO COLLISION " <<std::endl;
          printf("Time: %d, Node %d finished waiting for DIFS and started waiting for %d slots \n",dcf_wait_list[0].dcf_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].slots);
          printf("Time: %d, Node %d finished waiting and is ready to send the packet \n",dcf_wait_list[0].back_off_time , dcf_wait_list[0].pkt_id);
          printf("Time: %d, Node %d send %d \n",dcf_wait_list[0].end_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].pkt_size);
          // update the following information: new_time = dcf_wait_list[0].end_time + 1;

          for(size_t j = 1; j < dcf_wait_list.size(); j++){
            //std::cout << dcf_wait_list[j].sequence <<" SSSS " << std::endl;
            dcf_wait_list[j].new_time = dcf_wait_list[0].end_time + 1;
            _max = dcf_wait_list[0].end_time + 1;
            if(dcf_wait_list[j].back_off_time - dcf_wait_list[0].back_off_time < dcf_wait_list[j].slots){
              dcf_wait_list[j].slots = (dcf_wait_list[j].back_off_time - dcf_wait_list[0].back_off_time);
            }
          }
          sent_flag[dcf_wait_list[0].sequence] = true;


        }
        break;
      }
      for(size_t j = 0; j < dcf_wait_list.size(); j++){
        packet_out[dcf_wait_list[j].sequence] = dcf_wait_list[j];
        //std::cout <<i << " " <<  dcf_wait_list.size() << " " << dcf_wait_list[j].new_time << " XYZ " <<dcf_wait_list[j].sequence<< std::endl;
      }
         dcf_wait_list.erase(dcf_wait_list.begin());
      std::vector<packet>().swap(dcf_wait_list);
    }
     // for(size_t j = 0; j < dcf_wait_list.size(); j++){
     //    packet_out[dcf_wait_list[j].sequence] = dcf_wait_list[j];
     //  }

    i = i - back_up_size;
  }

}
void rts_mac_protocol(struct packet *packet_out,int no){
}
int main(int argc, char *argv[]) {
  int packet_num;
  int temp_no=0;
    //int method_no;
  std::string file_name;
  if(argc == 3){
    std::string name(argv[2]);
    file_name = name;
  }else{
    file_name = "data";
  }
  struct packet packte_out[1010];
  char strLine[1024];
  FILE *fp = fopen(file_name.c_str(), "rb");
  if( fp == NULL){
    printf("Error at opening files!\n");
    return 0;
  }
  fgets(strLine,1024,fp);
  packet_num = atoi(strLine);
  while (!feof(fp))
    {
      fgets(strLine,1024,fp);
      convert_int(strLine,&packte_out[temp_no]);
      temp_no++;
    }
  if(argc < 2){
    std::cout << "Must input one of the following simulator method: DCF, RTS " << std::endl;
    exit(1);
  }
  std::string method(argv[1]);
  if(method == "dcf"){
    dcf_mac_protocol(packte_out, packet_num);
  }else if(method == "rts"){
    rts_mac_protocol(packte_out, packet_num);
  }else{
    std::cout << "Wrong simulator method " << std::endl;
  }
  return 0;
}
