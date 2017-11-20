
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
  int rts_time;
  int cts_time;
  int NAV;
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
  printf("%d\n",no);
  //packet_out[0].slots=3;
  //packet_out[1].slots=14;
  //packet_out[2].slots=8;
  int temp=0;
  int temp1=0;
  int fraction=0;
  bool last_sent =  false;
  std::vector<packet>  dcf_wait_list, final_wait_list;
  std::vector<packet>  back_up;
  std::vector<bool> sent_flag(no,0);
  std::vector<int> vector_index;

  int _max = packet_out[0].new_time;
  for(int i = 0;i < no; i++){
    int diff = 0, jj = 0;
    //std::cout << "ITH HERE " << i <<" " << packet_out[i].new_time << " " << packet_out[i].pkt_id << " " << _max <<  std::endl;
    packet_out[i].sequence = i;
    _max = std::max(packet_out[i].new_time, _max);
    while(packet_out[i].new_time <= _max  && i < no){
      //std::cout << "ITH " << i <<" " << packet_out[i].new_time << " " << packet_out[i].pkt_id << " " << _max <<  std::endl;
      if(!sent_flag[i]){
        jj = 1;
        packet_out[i].sequence = i;
        printf("Time: %d, Node %d waiting for DIFS\n",packet_out[i].new_time,packet_out[i].pkt_id);
        packet_out[i].dcf_time = packet_out[i].new_time + DIFS;
        if(packet_out[i].slots == -1){
          if(!packet_out[i].collision){
            packet_out[i].slots = random_int(0, 15);
          }else{
            if(packet_out[i].repeat > 6){
                  temp1++;
                  sent_flag[i] = true;
                  printf("Time: %d, Node %d sent failed\n",packet_out[i].new_time,packet_out[i].pkt_id);
                  continue;
            }
            //std::cout<< " ith collision " << i << " " << packet_out[i].repeat << std::endl;
            packet_out[i].slots = random_int(0, 32 * (2^(packet_out[i].repeat-1)));
          }
        }
        packet_out[i].back_off_time = packet_out[i].dcf_time + SLOT * packet_out[i].slots;
        packet_out[i].end_time =packet_out[i].back_off_time + packet_out[i].trans_time;
        //printf("%d %d %d %d\n",packet_out[i].pkt_id,packet_out[i].dcf_time,packet_out[i].back_off_time,packet_out[i].end_time);
        dcf_wait_list.push_back(packet_out[i]);
        if(packet_out[i].back_off_time > _max){
          _max = packet_out[i].back_off_time;
        }
      }else{
        if(jj){
          diff++;
        }
      }
      i++;
      if(i == no) {
        i = i -1;
        break;
      }

    }

    std::sort(dcf_wait_list.begin(), dcf_wait_list.end(), [](struct packet left, struct packet right) { return (left.back_off_time) < (right.back_off_time);});
    int back_up_size = dcf_wait_list.size() + diff; // make backup
    if (back_up_size == 0) break;

    if(dcf_wait_list.size() == 1){
      printf("Time: %d, Node %d finished waiting for DIFS and started waiting for %d slots \n",dcf_wait_list[0].dcf_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].slots);
      printf("Time: %d, Node %d finished waiting and is ready to send the packet \n",dcf_wait_list[0].back_off_time, dcf_wait_list[0].pkt_id);
      printf("Time: %d, Node %d send %d \n",dcf_wait_list[0].end_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].pkt_size);
      temp++;
      //std::cout << " dcf_wait_list " << back_up_size << std::endl;
      sent_flag[dcf_wait_list[0].sequence] = true;
      if(no > 1){
        if(packet_out[i].new_time > dcf_wait_list[0].end_time){
          _max = packet_out[i].new_time;
        }else{
          _max = dcf_wait_list[0].end_time;
        }
      }
      //if( i == no -1) {_max = -1; last_sent = true;}
      //fraction=fraction+(_max-dcf_wait_list[0].end_time);
      //printf("%d %d %d\n",fraction,_max,dcf_wait_list[0].end_time);
      std::vector<packet>().swap(dcf_wait_list);
      int j = i;
      while(packet_out[j].new_time < _max){
        packet_out[j].new_time  = _max;
        j++;
      }
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
          }

          dcf_wait_list[ii].repeat += 1;
          dcf_wait_list[ii].collision = true;
          dcf_wait_list[ii].slots = -1; //random_int(0, 32 * dcf_wait_list[i].repeat);
          ii++;
          _collision = true; // collision detedte;
          //std::cout << " collision dectected"  << std::endl;
        }
        if(!_collision){
          //std::cout << "NO COLLISION " <<std::endl;
          printf("Time: %d, Node %d finished waiting for DIFS and started waiting for %d slots \n",dcf_wait_list[0].dcf_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].slots);
          printf("Time: %d, Node %d finished waiting and is ready to send the packet \n",dcf_wait_list[0].back_off_time , dcf_wait_list[0].pkt_id);
          printf("Time: %d, Node %d send %d \n",dcf_wait_list[0].end_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].pkt_size);
          temp++;
          //std::cout << " dcf_wait_list " << dcf_wait_list.size() + diff << std::endl;                    // update the following information: new_time = dcf_wait_list[0].end_time + 1;
          sent_flag[dcf_wait_list[0].sequence] = true;
          //if( i == no -1) {_max = -1; last_sent = true; break;}
          for(size_t j = 1; j < dcf_wait_list.size(); j++){
            if(dcf_wait_list[j].new_time < dcf_wait_list[0].end_time)
               dcf_wait_list[j].new_time = dcf_wait_list[0].end_time;

            _max = dcf_wait_list[0].end_time ;
            if(dcf_wait_list[j].back_off_time - dcf_wait_list[0].back_off_time < SLOT * dcf_wait_list[j].slots){
              dcf_wait_list[j].slots = dcf_wait_list[j].slots-((dcf_wait_list[0].back_off_time - dcf_wait_list[j].dcf_time))/9;
              //printf("%d\n",dcf_wait_list[j].slots);
            }else{
              dcf_wait_list[j].slots = -1;
            }
          }
        }
        break;
      }
      for(size_t j = 0; j < dcf_wait_list.size(); j++){
        //printf("%d\n", dcf_wait_list.size());
        packet_out[dcf_wait_list[j].sequence] = dcf_wait_list[j];

      }
      dcf_wait_list.erase(dcf_wait_list.begin());
      std::vector<packet>().swap(dcf_wait_list);
    }
      i = i - back_up_size -1;
      // if(i>no)
    //i=no-1;
  }


  printf("%d %d %d\n",temp,temp1,fraction);
  // std::cout << " ++++++++++++++++++++++ " << std::endl;
  // for(size_t i = 0; i < no; i++){
  //   std::cout << i << " " << sent_flag[i] << std::endl;
  // }

}
void rts_mac_protocol(struct packet *packet_out,int no){}
//   //packet_out[0].slots=3;
//   //packet_out[1].slots=14;
//     //packet_out[2].slots=8;
//   std::vector<packet>  dcf_wait_list;
//   std::vector<packet>  back_up;
//   std::vector<bool> sent_flag(no,0);
//   std::vector<int> vector_index;
//   int temp,temp1,fraction;
//   temp=temp1=fraction=0;
//   int _max = packet_out[0].new_time;
//   for(int i = 0;i < no; i++){
//     packet_out[i].sequence = i;
//     _max = std::max(packet_out[i].new_time, _max);
//     while(packet_out[i].new_time <= _max  && i < no){

//       if(!sent_flag[i]){
//         packet_out[i].sequence = i;
//         printf("Time: %d, Node %d waiting for DIFS\n",packet_out[i].new_time,packet_out[i].pkt_id);
//         packet_out[i].dcf_time = packet_out[i].new_time + DIFS;
//         if(packet_out[i].slots == -1){
//           if(!packet_out[i].collision){
//             packet_out[i].slots = random_int(0, 15);
//           }else{
//             if(packet_out[i].repeat > 10){
//               printf("Time: %d, Node %d sent failed\n",packet_out[i].new_time,packet_out[i].pkt_id);
//             }
//             packet_out[i].slots = random_int(0, 32 * packet_out[i].repeat);
//           }
//         }
//         packet_out[i].back_off_time = packet_out[i].dcf_time + SLOT * packet_out[i].slots;
//         packet_out[i].rts_time=packet_out[i].back_off_time+SIFS+RTS_CTS;
//         packet_out[i].cts_time=packet_out[i].rts_time+SIFS+RTS_CTS;
//         packet_out[i].end_time =packet_out[i].cts_time + packet_out[i].trans_time;
//         packet_out[i].NAV=packet_out[i].end_time-packet_out[i].back_off_time;
//         //printf("%d %d %d %d\n",packet_out[i].pkt_id,packet_out[i].back_off_time,packet_out[i].rts_time,packet_out[i].cts_time);
//         dcf_wait_list.push_back(packet_out[i]);
//         if(packet_out[i].back_off_time > _max){
//           _max = packet_out[i].back_off_time;
//         }
//       }
//       i++;
//       if(i == no) {
//         i = i -1;
//         break;
//       }

//     }
//     std::sort(dcf_wait_list.begin(), dcf_wait_list.end(), [](struct packet left, struct packet right) { return (left.back_off_time) < (right.back_off_time);});
//     int back_up_size = dcf_wait_list.size(); // make backup


//     if(dcf_wait_list.size() == 1){
//       printf("Time: %d, Node %d finished waiting for DIFS and started waiting for %d slots \n",dcf_wait_list[0].dcf_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].slots);
//       printf("Time: %d, Node %d finished waiting and is ready to send the RTS \n",dcf_wait_list[0].back_off_time, dcf_wait_list[0].pkt_id);
//       printf("Time: %d, Node %d finished waiting and is ready to receive the CTS \n",dcf_wait_list[0].rts_time, dcf_wait_list[0].pkt_id);
//       printf("Time: %d, Node %d finished waiting and is ready to send the packet \n",dcf_wait_list[0].cts_time, dcf_wait_list[0].pkt_id);
//       printf("Time: %d, Node %d send %d \n",dcf_wait_list[0].end_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].pkt_size);
//       temp++;
//       sent_flag[dcf_wait_list[0].sequence] = true;
//       if(no > 1){
//         _max = packet_out[dcf_wait_list[0].sequence + 1].new_time;
//       }
//       //fraction=fraction+(_max-dcf_wait_list[0].end_time);
//       std::vector<packet>().swap(dcf_wait_list);
//       i = i -1;
//       continue;
//     }
//     else{
//       bool _collision = false;
//       //std::cout << " HERE " << dcf_wait_list.size() << std::endl;
//       for(size_t ii = 1; ii < dcf_wait_list.size(); ii++){

//         while(dcf_wait_list[ii].back_off_time ==  dcf_wait_list[0].back_off_time&& ii < dcf_wait_list.size()){
//           if( ii == 1){
//             dcf_wait_list[0].repeat += 1;
//             dcf_wait_list[0].collision = true;
//             dcf_wait_list[0].slots = -1; //random_int(0, 32 * dcf_wait_list[0].repeat);
//           }
//           dcf_wait_list[ii].repeat += 1;
//           dcf_wait_list[ii].collision = true;
//           dcf_wait_list[ii].slots = -1; //random_int(0, 32 * dcf_wait_list[i].repeat);
//           ii++;
//           _collision = true; // collision detedte;
//           temp1++;
//         }
//         if(!_collision){
//           //std::cout << "NO COLLISION " <<std::endl;
//           printf("Time: %d, Node %d finished waiting for DIFS and started waiting for %d slots \n",dcf_wait_list[0].dcf_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].slots);
//           printf("Time: %d, Node %d finished waiting and is ready to send the RTS \n",dcf_wait_list[0].back_off_time, dcf_wait_list[0].pkt_id);
//           printf("Time: %d, Node %d finished waiting and is ready to receive the CTS \n",dcf_wait_list[0].rts_time, dcf_wait_list[0].pkt_id);
//           printf("Time: %d, Node %d finished waiting and is ready to send the packet \n",dcf_wait_list[0].cts_time , dcf_wait_list[0].pkt_id);
//           printf("Time: %d, Node %d send %d \n",dcf_wait_list[0].end_time, dcf_wait_list[0].pkt_id, dcf_wait_list[0].pkt_size);
//           temp++;
//           // update the following information: new_time = dcf_wait_list[0].end_time + 1;

//           for(size_t j = 1; j < dcf_wait_list.size(); j++){
//             dcf_wait_list[j].new_time = dcf_wait_list[0].end_time;
//             _max = dcf_wait_list[0].end_time ;
//             if(dcf_wait_list[j].back_off_time - dcf_wait_list[0].back_off_time < SLOT * dcf_wait_list[j].slots){
//               dcf_wait_list[j].slots = dcf_wait_list[j].slots-((dcf_wait_list[0].back_off_time - dcf_wait_list[j].dcf_time))/9;
//               //printf("%d\n",dcf_wait_list[j].slots);
//             }else{
//               dcf_wait_list[j].slots = -1;
//             }
//           }
//           sent_flag[dcf_wait_list[0].sequence] = true;


//         }
//         break;
//       }
//       for(size_t j = 0; j < dcf_wait_list.size(); j++){
//         //printf("%d\n", dcf_wait_list.size());
//         packet_out[dcf_wait_list[j].sequence] = dcf_wait_list[j];

//       }
//       dcf_wait_list.erase(dcf_wait_list.begin());
//       std::vector<packet>().swap(dcf_wait_list);
//     }
//     //printf("%d %d \n",i,back_up_size);
//     i = i - back_up_size;
//         //printf("%d\n",i);
//   }
//   printf ("%d %d %d",temp, temp1, fraction);
//   // std::cout << " ++++++++++++++++++++++ " << std::endl;
//   // for(size_t i = 0; i < no; i++){
//   //   std::cout << i << " " << sent_flag[i] << std::endl;
//   // }

// }
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
  struct packet packte_out[100010];
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
