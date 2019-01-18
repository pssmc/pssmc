#include "mutation.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
int mutype;
#define BUFLEN 255 
#define MAX_LINE 1024 

extern int numOfBSpec;
extern double* coefOfBSpec;
extern int numOfSpec;

int main(int argc,char *argv[])
{
	time_t t1 = time( 0 );  
    char tmpBuf_begin[BUFLEN];  
    strftime(tmpBuf_begin, BUFLEN, "%Y-%m-%d %H:%M:%S", localtime(&t1));

	srand(time(0));	
	initInputParamenters();
	int numofspin = 0,numofiteration = 0;

	int numofcandidate = 100,maxfitness = 100;
	int numofsolution = 0;
	program** candidate = genInitTemplate(numofcandidate);

	int i,j,k;
	
	Expr** requirements = (Expr**)malloc(sizeof(Expr*) * numOfBSpec);

	for(i = 0;i < numOfBSpec;i++)
	{
		FILE* fp = NULL;
		char buf[255];
		char name[50] = "../property/BSpec";
		char iString[10];
		sprintf(iString,"%d",i);
		strcat(name,iString);
		strcat(name,".bltl");
		fp = fopen(name,"r");break;

		
		fgets(buf, 255, fp);
		
		fclose(fp);
		int index = 0;
		j = 0;
		while(buf[j] != '\0')
		{
			if(buf[j] != ' ' && buf[j] != '\n')
			{
				buf[index] = buf[j];
				index++;
			}
			j++;
		}
		buf[index] = '\0';
	
		Expr* e = generateStepExpr(buf,0,strlen(buf) - 1);
		requirements[i] = e;
		printExpr(e);
		printf("\n");
	}

	int maxstepbound = 0;
	for(i = 0;i < numOfBSpec;i++)
	{
		int step = getStepBound(requirements[i]);
		if(step > maxstepbound)
			maxstepbound = step;
	}
	initTraceGlobalVar(maxstepbound + 1);

	for(i = 0;i < numofcandidate;i++)
	{	
		organism* org = genOrganism(candidate[i]);
		double candidatefit = calculateFitness(org,requirements,numOfBSpec,coefOfBSpec);
		candidate[i]->fitness = candidatefit;
		for(j = 0;j < numOfBSpec;j++)
			candidate[i]->propertyfit[j] = org->progs[0]->propertyfit[j];
		freeOrganism(org);
	}

	int right = 0,maxiteration = 2000;
	double m = 0.3;
	for(i = 0;i < maxiteration;i++)
	{	printf("1");
		int count = 0;
		program** newcandidate = (program**)malloc(sizeof(program*) * numofcandidate);

		int* fit = (int*)malloc(sizeof(int) * numofcandidate);
		int sumfitness = 0;
		for(j = 0;j < numofcandidate;j++)
		{
			fit[j] = candidate[j]->fitness;
			sumfitness += fit[j];
		}

		int* roulette = (int*)malloc(sizeof(int) * sumfitness);
		int tempfit = 0;
		for(j = 0;j < numofcandidate;j++)
			for(k = 0;k < fit[j];k++)
			{
				roulette[tempfit++] = j;
			}

		while(count < numofcandidate)
		{
			int index = roulette[nextrand(sumfitness)];

			if(nextrand(100) < m * 100)
			{	//printf("%d,mutation.",count);
				//printf("2");
				newcandidate[count] = mutation(candidate[index]);
				//printf("3");				
				setAll(newcandidate[count]);
				organism* org = genOrganism(newcandidate[count]);
	
				double candidatefit = calculateFitness(org,requirements,numOfBSpec,coefOfBSpec);
				newcandidate[count]->fitness = candidatefit;
				freeOrganism(org);
			}			
			else
			{	
				newcandidate[count] = copyProgram(candidate[index]);
				setAll(newcandidate[count]);
				newcandidate[count]->fitness = candidate[index]->fitness;
				for(j = 0;j < numOfBSpec;j++)
					newcandidate[count]->propertyfit[j] = candidate[index]->propertyfit[j];
			}
			count++;							
		}
		
		double maxf = 0;
		for(j = 0;j < numofcandidate;j++)
			if(newcandidate[j]->fitness > maxf)
				maxf = newcandidate[j]->fitness;
		printf("iteration:%d,max fitness = %lf\n",i,maxf);
		
		for(j = 0;j < numofcandidate;j++)
			freeProgram(candidate[j]);
		free(candidate);
		candidate = newcandidate;


		for(j = 0;j < numofcandidate;j++)
		{
			if(candidate[j]->fitness > 98 && candidate[j]->checkedBySpin == 0)	
			{	
				candidate[j]->checkedBySpin = 1;
				organism* org = genOrganism(candidate[j]);

				FILE* f;
				char filename[30] = "../output/mutex";
				strcat(filename,argv[1]);
				strcat(filename,".pml");
				if(f = fopen(filename,"w"))
				{	
					orgToPml(org,f);
				}	
				//sleep(2);
				fclose(f);
				
				char command[50] = "spin -a ";
				strcat(command,filename);
				strcat(command," > useless");
				
				system(command);
				system("gcc -DMEMLIM=1024 -O2 -DXUSAFE -w -o pan pan.c");
				
				int k;
				for(k = 0;k < numOfSpec;k++)
				{
					numofspin++;
					char spinCommand1[128];
					sprintf(spinCommand1,"./pan -m10000 -a -f -N e%d > pan%d.out",k + 1,k + 1);
					system(spinCommand1);
					char spinCommand2[128];
					sprintf(spinCommand2,"grep -q -e \"errors: 0\" pan%d.out",k + 1);
					int r = system(spinCommand2);
					if(r != 0)
						continue;

				}					
				//if(r3 == 0)	candidate[j]->fitness += 5;		
				freeOrganism(org);
			}
		}

		if(right == 1)
			break;
	}
	numofiteration = i;

	time_t t2 = time( 0 );  
    char tmpBuf_end[BUFLEN];  
    strftime(tmpBuf_end, BUFLEN, "%Y-%m-%d %H:%M:%S", localtime(&t2));

	FILE* f2;
	char filename[30] = "../log";
	if(f2 = fopen(filename,"ab+"))
	{	
		fputs("start: ",f2);
		fputs(tmpBuf_begin,f2);
		fputs(";end: ",f2);
		fputs(tmpBuf_end,f2);
		fputs(";call of spin: ",f2);
		fprintf(f2,"%d",numofspin);
		fputs(";numofiteration = ",f2);
		fprintf(f2,"%d",numofiteration);		
		fputs("\n",f2);
	}
	return right;
}
