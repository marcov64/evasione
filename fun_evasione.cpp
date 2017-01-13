#include "fun_head.h"

MODELBEGIN


EQUATION("Purchase")
/*
Choose one product and assign it to the consumer, pointed to by c->

The choice is probabilistic, depending on the acceptance of tax cheating firms by the consumer and on the Utility of the product offered by the sellers.

The equation also register the sale of the selected firm.
*/

v[0]=VS(c,"ProbAcceptEvasion");

v[10]=V("ExpPurchase");
if(v[0]>RND)
 v[1]=1;
else
 v[1]=0;

v[15]=0;
CYCLES(c, cur, "Supplier")
 {
  v[2]=VS(cur->hook,"Evading");
  if(v[1]==0 && v[2]==1)
   WRITES(cur,"sapp",0);
  else 
   {
    v[3]=VS(cur->hook,"Utility");  
    WRITES(cur,"sapp",pow(v[3],v[10]));
    v[15]++;
   } 
 }
if(v[15]>0)
 cur=RNDDRAWS(c,"Supplier","sapp");
else
 cur=RNDDRAWFAIRS(c,"Supplier");

c->hook=cur->hook;
v[4]=VS(cur->hook,"idFirm");
WRITES(c,"ProdUsed",v[4]);
INCRS(cur->hook,"Sales",1);

v[5]=VS(cur->hook,"Utility");
v[6]=V("aUtility");
v[7]=VS(c,"ConsumerWelfare");
v[8]=v[6]*v[7]+(1-v[6])*v[5];
WRITES(c,"ConsumerWelfare",v[8]);

RESULT(1 )

EQUATION("ProbAcceptEvasion")
/*
Probability to include suppliers evading taxes within the option set for potential purchaes.

It is computed as a slow approximation to the average probability of evading from its own experience and that from other consumers.
*/

v[10]=V("ProbConsumerControl");
if(RND<v[10])
 END_EQUATION(0);
 
v[0]=VL("ProbAcceptEvasion",1);
v[1]=V("aProbEvasion");

v[6]=v[1]*v[0]+(1-v[1]);//automatic increase


v[2]=v[3]=0;

CYCLE(cur, "cLink")
 {
  v[2]+=VLS(cur->hook,"ProbAcceptEvasion",1);
  v[3]++;
 }

v[4]=v[2]/v[3];

v[7]=V("WeightDirectExperience");
v[5]=v[7]*v[6]+(1-v[7])*v[4];
RESULT( v[5])


EQUATION("Utility")
/*
Utility for the consumer by choosing one product, mix of standard utility plus, in case of evasion, the difference of the unpaid tax and the extra gain for the producer.
*/

v[0]=V("BasicUtility");
v[1]=V("UnitTax");
v[2]=V("UnitGainEvasion");
v[3]=V("Evading");

v[4]=v[0]+v[3]*(v[2]-v[1]);
RESULT(v[4] )


EQUATION("GovernmentUtility")
/*
Utility for the government measured after trading is completed.
*/

V("Shopping"); //ensure consumers made their choices

v[0]=v[1]=v[2]=v[3]=0;
v[11]=V("CostControl");
v[13]=V("UnitFine");
v[18]=V("ProbControl");
CYCLE(cur,"Firm")
 { //for all firms
  if(RND<v[18])
   {//pick some randomly
    v[0]+=v[11];
    v[1]++;
    WRITES(cur,"Controlled",1); 
    v[12]=VS(cur,"Evading");
    if(v[12]==1)
     {//if the controlled firm is evading than ..
      WRITES(cur,"Punished",v[15]);
      v[2]++;
      v[14]=VS(cur,"Sales");
      v[3]+=v[14]*v[13];
     }
   }
   else
     WRITES(cur,"Controlled",0); 
 }

WRITE("TotalCost",v[1]*v[11]);
WRITE("TotalFine",v[3]);
WRITE("TotalControls",v[1]);
WRITE("TotalCaughtEvading",v[2]);
RESULT(v[3]-v[1]*v[11] )


EQUATION("MemoryControlled")
/*
Element set to 1 if the firm has just undergone a gov. control. The memory fades trhough time, though can be revivied up by observing other firms being controlled.
*/

v[0]=VL("MemoryControlled",1);
v[1]=V("aMemControlled");

v[2]=v[0]*v[1];

CYCLE(cur, "fLink")
 {//check whether the last time step connected firms were evading or not, weitghted by their size
  v[11]=VLS(cur->hook,"AvSales",1);
  v[1]+=VS(cur->hook,"Controlled")*v[11];
  v[2]+=v[11];  
 }

v[3]=v[1]/v[2]; //share of linked firms being controlled last time 


v[5]=V("weightPersonalMemory");
	
v[6]=v[5]*v[2]+(1-v[5])*v[3]; //perception of prob. of being controlled, avereged between observation and personal memory


RESULT(v[6] )


EQUATION("Evading")
/*
Flag signaling whether the firm cheats on taxes (1) or pays (0)


Otherwise, it uses an estimation of the gain from paying taxes and not paying them, the discounted by the observed risk of being caught.
*/

v[6]=V("MemoryControlled"); //Parameter set to 1 if the firm has just undergone a gov. control and 0 expecting no controls

 
v[7]=V("UnitFine");
v[15]=V("UnitProfit");
v[8]=V("UnitGainEvasion");
v[9]=V("LumpPunishment");
//v[10]=VL("AvSalesEvading",1);
//v[14]=VL("AvSalesNoEvading",1);

v[10]=v[14]=VL("AvSales",1);
v[11]=(v[9]+v[10]*v[7]); //expected costs of evading
v[12]=v[10]*(v[8]+v[15]) - v[11]; //net gains from evading

v[16]=v[14]*v[15]; //gains from not evading
if(v[12]*(1-v[6]) > v[16])
 v[13]=1;
else
 v[13]=0; 
WRITE("GainEvading",v[12]);
WRITE("CostEvading",v[16]);

RESULT(v[13] )


EQUATION("AvSales")
/*
Compute the moving average of sales
*/

V("Shopping");
v[0]=VL("AvSales",1);
v[1]=V("aAvSales");
v[2]=V("Sales");

v[3]=v[0]*v[1]+(1-v[1])*v[2];

RESULT(v[3] )


EQUATION("AvSalesEvading")
/*
Estimate of moving average of sales when evading
*/

v[0]=v[1]=0;

CYCLE(cur, "fLink")
 {
  v[0]+=VLS(cur->hook,"AvSalesEvading",1);
  v[1]++;
 }

v[2]=v[0]/v[1];
v[5]=V("weightPersonalMemory");
v[6]=V("aAvSales");
v[7]=V("Sales");
v[4]=V("Evading");
if(v[4]==1)
  v[8]=v[5]*v[7]+(1-v[5])*v[2];
else
  v[8]=v[2]; 

v[9]=v[6]*CURRENT+(1-v[6])*v[8]; 
RESULT(v[9])


EQUATION("AvSalesNoEvading")
/*
Estimate of moving sales when not evading
*/

v[0]=v[1]=0;

CYCLE(cur, "fLink")
 {
  v[0]+=VLS(cur->hook,"AvSalesNoEvading",1);
  v[1]++;
 }

v[2]=v[0]/v[1];
v[5]=V("weightPersonalMemory");
v[6]=V("aAvSales");
v[7]=V("Sales");
v[4]=V("Evading");
if(v[4]==0)
  v[8]=v[5]*v[7]+(1-v[5])*v[2];
else
  v[8]=v[2]; 
v[9]=v[6]*CURRENT+(1-v[6])*v[8]; 
RESULT( v[9])

EQUATION("Profit")
/*
Profit are proportional to sale. If the firm evades taxes it also gains from a share of unpaid tax. In case of being caught it pays the fine.

The fine is computed on each single sale. Additionally, a lump punishment, such as a prison term, is added to the penalty.
*/
V("Shopping");
v[0]=V("Sales");
v[1]=V("Evading");
v[2]=V("UnitProfit");
v[3]=V("UnitGainEvasion");
v[5]=V("Punished");
v[6]=V("UnitFine");
v[7]=V("LumpPunishment");
v[4]=v[0]*(v[2]+v[3]*v[1] - v[5]*v[6] ) - v[7]*v[5];
RESULT(v[4] )

EQUATION("Shopping")
/*
Round of purchase for each consumer. After purchasing, the government may control whether the supplier has evaded and punishes the consumer cutting setting the flag Fined=1
*/

CYCLE(cur, "Firm")
  WRITES(cur,"Sales",0);

v[0]=V("ProbConsumerControl");
v[1]=0;
CYCLE(cur, "Consumer")
 {
  V_CHEAT("Purchase", cur);
  if(RND<v[0])
   {v[1]++;
    if(VS(cur->hook, "Evading")==1)
     WRITES(cur,"Fined",1);
    else
     WRITES(cur,"Fined",0);;     
   }
 }

RESULT(v[1])

EQUATION("Statistics")
/*
Stastics
*/

v[0]=v[1]=v[2]=v[3]=v[4]=0;
CYCLE(cur, "Firm")
 {
 v[0]++;
 v[1]+=VS(cur,"Evading");
 v[2]+=VS(cur,"Profit");
 v[3]+=VS(cur,"MemoryControlled");
 }

WRITE("AvEvading",v[1]/v[0]);
WRITE("AvProfit",v[2]/v[0]);
WRITE("AvMemControlled",v[3]/v[0]);

RESULT(1 )


EQUATION("Init")
/*
Initialize the model
*/

V("InitSupply");
V("InitDemand");
PARAMETER
RESULT(1 )


EQUATION("InitSupply")
/*
Create a network of firms connected randomly to each other.
*/
v[0]=V("numFirms"); //desired number of nodes
ADDNOBJ("Firm",v[0]-1);
v[1]=1;
CYCLE(cur, "Firm")
  {
   WRITES(cur,"idFirm",v[1]++); //assign a unique id to nodes
   WRITELS(cur,"Evading",0,t-1);
   WRITELS(cur,"AvSalesEvading",1,t-1);
   WRITELS(cur,"AvSalesNoEvading",1,t-1);
   WRITELS(cur,"AvSales",1,t-1);
   WRITELS(cur,"Utility",0,t-1);
  } 

p->initturbo("Firm", v[0]); //initialize the turbo structure
v[1]=V("numfLinks");

CYCLE(cur, "Firm") //for each node
 {//WRITES(cur,"nLinks",v[1]);
  ADDNOBJS(cur,"fLink",v[1]-1); //add objects Link not created before
  v[3]=VS(cur,"idFirm");  //register the id of the current node
  CYCLES(cur, cur1, "fLink")
   { //for each link
    v[2]=v[3]; //force the initial computation
    while( v[2]==v[3] )
     {//repeat to avoid duplication or self-links
      v[2]=rnd_integer(1,v[0]);
      if(v[2]!=v[3])
      {
       CYCLES(cur, cur2, "fLink")
        {
         if(VS(cur2, "idfLink")==v[2])
          {//if a link already exist fake the drawing of itself
           v[2]=v[3];
           break;
          }
        }
       } 
     };     
    WRITES(cur1,"idfLink",v[2]);
    cur3=p->turbosearch("Firm", v[0],v[2]);//search the actual node corresponding to v[2]
    cur1->hook=cur3;
   }
  SORTS(cur,"fLink","idfLink", "UP");
 }
PARAMETER
RESULT( 1)



EQUATION("InitDemand")
/*
Create a network of consumers connected randomly to each other.

Moreover, each consumer is endowed with a number of suppliers, choosing randomly, from which it will have to choose.
*/
v[0]=V("numConsumer"); //desired number of nodes
ADDNOBJ("Consumer",v[0]-1);
v[1]=1;
CYCLE(cur, "Consumer")
  WRITES(cur,"idConsumer",v[1]++); //assign a unique id to nodes

p->initturbo("Consumer", v[0]); //initialize the turbo structure
v[1]=V("numcLinks");
v[10]=V("numSuppliers");

CYCLE(cur, "Consumer") //for each node
 {//WRITES(cur,"nLinks",v[1]);
  ADDNOBJS(cur,"cLink",v[1]-1); //add objects Link not created before
  v[3]=VS(cur,"idConsumer");  //register the id of the current node
  CYCLES(cur, cur1, "cLink")
   { //for each link
    v[2]=v[3]; //force the initial computation
    while( v[2]==v[3] )
     {//repeat to avoid duplication or self-links
      v[2]=rnd_integer(1,v[0]);
      if(v[2]!=v[3])
      {
       CYCLES(cur, cur2, "cLink")
        {
         if(VS(cur2, "idcLink")==v[2])
          {//if a link already exist fake the drawing of itself
           v[2]=v[3];
           break;
          }
        }
       } 
     };     
    WRITES(cur1,"idcLink",v[2]);
    cur3=p->turbosearch("Consumer", v[0],v[2]);//search the actual node corresponding to v[2]
    cur1->hook=cur3;
   }
  SORTS(cur,"cLink","idcLink", "UP");
  ADDNOBJS(cur,"Supplier",v[10]-1);
  v[11]=V("numFirms");
  CYCLES(cur, cur1, "Supplier")
   { //for each link
    v[2]=-1; //force the initial computation
    while( v[2]==-1 )
     {//repeat to avoid duplication or self-links
      v[2]=rnd_integer(1,v[11]);
      if(v[2]!=-1)
      {
       CYCLES(cur, cur2, "Supplier")
        {
         if(VS(cur2, "idSupplier")==v[2])
          {//if a link already exist fake the drawing of itself
           v[2]=-1;
           break;
          }
        }
       } 
     };     
    WRITES(cur1,"idSupplier",v[2]);
    cur2=SEARCH_CND("idFirm",v[2]);
    cur1->hook=cur2;
   } 
  SORTS(cur,"Supplier","idSupplier", "UP");
 }
PARAMETER
RESULT( 1)

MODELEND




void close_sim(void)
{

}


