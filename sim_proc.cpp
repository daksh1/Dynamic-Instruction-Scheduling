#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include<iostream>
using namespace std;
struct ReorderBuffer {
        int state; //IF:1  ID:2  IS:3  EX:4  WB:5
        int source1_state, source2_state, operand_state; 
        int tag;
        int dispatch_list;
        int issue_list;
        int execute_list; 
        int valid;
        unsigned int function_type;
        int src1, src2, dst;
        unsigned int if_cycle, if_dur;
        unsigned int id_cycle, id_dur;
        unsigned int is_cycle, is_dur;
        unsigned int ex_cycle, ex_dur;
        unsigned int wb_cycle, wb_dur;
        unsigned int i, cycle;
        unsigned int count_ex; 
        int entry;
        int src1_depends_on_this_entry, src2_depends_on_this_entry;
        ReorderBuffer * nextrob;
        ReorderBuffer * lastrob;
};
struct RegisterFile {
        int tag;
        int valid;
};
void printInstruction(ReorderBuffer *a, int clk_cycle) {
        cout<<"Clock: "<<clk_cycle<<" Tag: "<<a->tag<<" stage: "<<a->state<<" src1: "
        <<a->src1<<" sr2: "<<a->src2<<" dst: "<<a->dst<<" function_type: "
        <<a->function_type<<" operand_state: "<<a->operand_state<<"\n";
}
int main(int argc, char * argv[]) {
        int i, j;
        FILE * tracefile;
        string tracename;
        string seq_no;
        string mem_no;
        int op, dst, src1, src2;
        int final_cycle = 0;
        int tag = 0;
        int N, S;
        int clk_cycle;
        int count_rob, count_rob_id; 
        int count_FU, count_issue;
        static RegisterFile rf[128]; //Register File
        double IPC;
        ReorderBuffer rob[1024];
        ReorderBuffer * head, * temprob, * temprob2, * tail;
        head = rob;
        tail = head;
        rob[1023].nextrob = & rob[0];
        rob[0].lastrob = & rob[1023];
        int numbers_printed= 0;
       //Initialize
        
        for (i = 0; i < 1023; i++) {
                rob[i + 1].lastrob = & rob[i];
                rob[i].nextrob = & rob[i + 1];
        }
        for (i = 0; i < 1024; i++) {
                rob[i].valid = 1;
                rob[i].dispatch_list = 0;
                rob[i].issue_list = 0;
                rob[i].execute_list = 0;
                rob[i].entry = i;
                rob[i].operand_state = 1;
        }
        for (j = 0; j < 128; j++) {
                rf[j].valid = 1;
        }
        clk_cycle = 0;
        count_rob = 0;
        count_rob_id = 0;
        
       //Read_In File
        
        S = stoi(argv[1]); 
        N = stoi(argv[2]); 
        tracefile = fopen(argv[8], "r");
        
        if (tracefile == NULL) {
            return 0;
        }
        count_issue = S;
        count_FU = N + 1;
        count_rob = N;
        count_rob_id = N * 2;
        temprob2 = head;
        head->lastrob = NULL;
        int issue_rate = 0;
       
        //Cycle begins
        
        while (numbers_printed< 10000) {
                issue_rate = N;
                //   1.FakeRetire(); WB-> OUT
                for (temprob = head; temprob != tail; temprob = temprob->nextrob) {
                        if (head->state == 6) head = head->nextrob;
                }
                for (temprob = head; temprob != tail; temprob = temprob->nextrob) {
                        if (temprob->state == 5) //reached WB, completed
                        {
                                i++;
                                temprob->state = 6; 
                                temprob->valid = 0;
                                temprob->wb_dur =1;
                                if (temprob2->state == 6 && temprob2->tag == numbers_printed&& numbers_printed< 10000)
                                {
				        printf("%d fu{%d} src{%d,%d} dst{%d} IF{%d,%d} ID{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d}\n",temprob2->tag,temprob2->function_type,temprob2->src1, temprob2->src2,temprob2->dst,temprob2->if_cycle, temprob2->if_dur,
																												temprob2->id_cycle, temprob2->id_dur,temprob2->is_cycle, temprob2->is_dur,temprob2->ex_cycle, temprob2->ex_dur,
																												temprob2->wb_cycle, temprob2->wb_dur);
								
                                        if (final_cycle < temprob2->wb_cycle + temprob2->wb_dur)
                                                final_cycle = temprob2->wb_cycle + temprob2->wb_dur;
                                        numbers_printed++;
                                        temprob2 = temprob2->nextrob;
                                }
                        }
                }
                //     2.Execute(); EX-> WB
                
                for (temprob = head; temprob != tail; temprob = temprob->nextrob) {
                        if(temprob->tag == 4 && temprob->state == 4)
                                cout<<"";
                        if (!temprob->count_ex && temprob->state == 4) {
                                        temprob->execute_list = 0;
                                        temprob->state = 5; 
                                        count_FU++;
                                        if (temprob->dst != -1) {
                                                if (temprob->entry == rf[temprob->dst].tag) {
                                                        rf[temprob->dst].valid = 1; //tag match
                                                }
                                                // Update Existing Source States
                                                
                                                for (ReorderBuffer *x = head; x != tail; x = x->nextrob) { 
                                                        
                                                        if(x->state != 3 && x == temprob)
                                                                continue;         

                                                        if(x->tag == 4)
                                                                cout<<"";

                                                        if(temprob->tag==x->src1_depends_on_this_entry)
                                                                x->source1_state=1;
                                                        if(temprob->tag==x->src2_depends_on_this_entry)
                                                                x->source2_state=1;
                                                }
                                               
                                        }
                                        temprob->operand_state = 1; //operand ready
                                        temprob->wb_cycle = clk_cycle;
                                        temprob->ex_dur = (temprob->wb_cycle - temprob->ex_cycle);
                                        
                        } else if (temprob->state == 4)
                                temprob->count_ex--; 
                }
                //   3.Issue();IS-> EX

                for (temprob = head; temprob != tail; temprob = temprob->nextrob) {
                        if (temprob->state == 3 && issue_rate) {       
                                if ((rob[temprob->src1_depends_on_this_entry].operand_state || temprob->source1_state) && (rob[temprob->src2_depends_on_this_entry].operand_state || temprob->source2_state)) {
                                        issue_rate--;
                                        count_FU--;
                                        temprob->count_ex--;
                                        temprob->issue_list = 0;
                                        temprob->execute_list = 1;
                                        temprob->state = 4; 
                                        count_issue++;
                                        temprob->ex_cycle = clk_cycle;
                                        temprob->is_dur = (temprob->ex_cycle - temprob->is_cycle);
                                }
                        }
                }
                //   4.Dispatch(); ID->IS
               // is the scheduling queue full ?
                for (temprob = head; temprob != tail; temprob = temprob->nextrob) {
                        if (count_issue && temprob->state == 2) {
                                count_issue--;
                                count_rob_id++;
                                temprob->dispatch_list = 0;
                                temprob->issue_list = 1;
                                temprob->state = 3; 
                                temprob->is_cycle = clk_cycle;
                                temprob->id_dur = (temprob->is_cycle - temprob->id_cycle);
                        }
                }
                //   5.Fetch(); IF->ID
                for (temprob = head; temprob != tail; temprob = temprob->nextrob) {
                        if (count_rob_id && temprob->state == 1) {
                                temprob->dispatch_list = 1;
                                count_rob++;
                                temprob->state = 2; 
                        }
                }
                //   6. IN ->IF
                while (count_rob && count_rob_id) {
                        count_rob--;
                        count_rob_id--;
			fscanf(tracefile,"%s %d %d %d %d %s\n",&seq_no[0], &op,&dst,&src1,&src2, &mem_no[0]);
                        tail->function_type = op;
                        if (tail->function_type == 0) tail->count_ex = 1;
                        else if (tail->function_type == 1) tail->count_ex = 2;
                        else if (tail->function_type == 2) tail->count_ex = 5;
                        tail->tag = tag;
                        tail->src1 = src1;
                        tail->src2 = src2;
                        tail->dst = dst;
                        tail->if_cycle = clk_cycle;
                        tail->id_cycle = clk_cycle + 1;
                        tail->if_dur = 1;
                        tail->state = 1; 
                        tail->operand_state = 0; //not completed
                        if (!rf[tail->src1].valid && tail->src1 != -1) {
                                tail->src1_depends_on_this_entry = rf[tail->src1].tag;
                                tail->source1_state = 0; //occupied by others
                        } else if (rf[tail->src1].valid || tail->src1 == -1) {
                                tail->source1_state = 1;
                                tail->src1_depends_on_this_entry = tail->entry;
                        }
                        if (!rf[tail->src2].valid && tail->src2 != -1) {
                                tail->src2_depends_on_this_entry = rf[tail->src2].tag; 
                                tail->source2_state = 0;
                        } else if (rf[tail->src2].valid || tail->src2 == -1) {
                                tail->source2_state = 1;
                                tail->src2_depends_on_this_entry = tail->entry;
                        }
                        rf[tail->dst].tag = tail->entry;
                        rf[tail->dst].valid = 0;  
                        tag++;
                        tail = tail->nextrob;                  
                }
                clk_cycle++;
        }
        IPC = (double) numbers_printed/ (double) final_cycle;
        printf("CONFIGURATION\n");
        printf("superscalar bandwidth (N) = %d\ndispatch queue size (2*N) = %d\nschedule queue size (S) = %d\n", N, 2*N, S);
        printf("RESULTS\n");
        printf("number of instructions = %d\nnumber of cycles       = %d\nIPC                    = %0.2f", numbers_printed, final_cycle, IPC);
        fclose(tracefile);
        return (0);
}