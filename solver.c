#include <stdio.h>
#include <stdlib.h>
int solverJedanD(int s[81]){ //primi niz od 81 element
int r=99;//da zabiljezi kolnu u kojoj je 0
int k=99;//da zabiljezi red u kojoj je 0
int broj=99;//d zabiljezi redni broj u nizu gdje je nula tipa ako je nula naniz[43] onda je broj=43
int i,j,m=0;
for(i=0;i<9;i++){ //petlja za trazenje nule
    for(j=0;j<9;j++){
        if(s[m]==0){
            r=i;
            k=j;
            broj=m;
        }
        m++;
    }
}
if(r==99 && k==99 && broj==99)return 1;
for(j=1;j<10;j++){
    if(provjeriKockujedanD(j,r,k,s)==0 && provjeriKolonujednaD(j,r,k,s)==0 && provjeriRedjedanD(j,r,k,s)==0){ //za svih 9 brojeva provjerava mogul se ubacit
        s[broj]=j;//prvi koji moze ubaci
        if(solverJedanD(s)==1)return 1;//rekurzivnim putem ako se ne moze ubacit vraca se nazad
        s[broj]=0;
    }
}
return 0;
}

int provjeriRedjedanD(int broj,int red,int kolona,int s[81]){
    int i;
    for(i=0;i<9;i++){
        if(s[i+red*9]==broj)
            return 1;
    }
     return 0;
}
int provjeriKolonujednaD(int broj,int red,int kolona,int s[81]){
    int i;
    for(i=0;i<9;i++){
        if(s[i*9+kolona]==broj)
            return 1;
    }
     return 0;
}

int provjeriKockujedanD(int broj,int red, int kolona,int s[81]){
    int i,j;
    //samo se jedan od uslova izvrsi prilikom poziva funkcije tako da je O( n na 2)
    //uradit cu ovu funkciju krace ovo mi je prvo palo napamet
    if(red<3 && kolona<3){//provjeri jel  celija u prvoj kocki
        for(i=0;i<3;i++){
            for(j=0;j<3;j++){
                if(s[j+i*9]==broj)
                    return 1;
            }
        }
    return 0;
    }
    else if (red<3 && kolona>=3 && kolona<6){//provjeri jel u drugoj kocki
        for(i=0;i<3;i++){
            for(j=3;j<6;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
    else if( red<3 && kolona>=6 && kolona<9){//provjeri jel u trecoj kocki
        for(i=0;i<3;i++){
            for(j=6;j<9;j++){
                if(s[j+i*9]==broj)
                    return 1;
            }
        }
    return 0;
    }
    else if( red>=3 && red<6 && kolona<3 ){//provjeri jel u cetvrtoj kocki
        for(i=3;i<6;i++){
            for(j=0;j<3;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
    else if( red>=3 && red<6 && kolona>=3 &&kolona<6){//provjeri jel u petoj kocki
        for(i=3;i<6;i++){
            for(j=3;j<6;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
    else if( red>=3 && red<6 && kolona>=6 &&kolona<9){//provjeri jel u sestoj kocki
        for(i=3;i<6;i++){
            for(j=6;j<9;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
    else if( red>=6 && red<9 && kolona<3){//provjeri jel u sedmoj kocki
        for(i=6;i<9;i++){
            for(j=0;j<3;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
    else if( red>=6 && red<9 && kolona>=3 &&kolona<6){//provjeri jel u osmoj kocki
        for(i=6;i<9;i++){
            for(j=3;j<6;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
    else if( red>=6 && red<9 && kolona>=6 &&kolona<9){//provjeri jel u posljednjoj devetoj kocki
        for(i=6;i<9;i++){
            for(j=6;j<9;j++){
                if(s[j+i*9]==broj)return 1;
            }
        }
    return 0;
    }
}
int main()
{
    int niz[81]={3,7,0,0,0,0,0,0,1,0,0,0,7,0,0,0,0,5,4,0,8,0,6,1,0,9,0,0,0,0,0,1,0,0,0,0,0,5,0,0,9,0,4,6,0,0,8,6,0,0,2,0,3,0,0,0,0,0,0,0,0,0,0,6,9,4,0,0,5,2,0,3,8,0,0,1,4,9,5,0,0};
    int i;
    for(i=0;i<81;i++)
        {
            if(i%9==0)printf("\n");
            if(niz[i]==0)printf("-");
           else printf("%i ",niz[i]);
        }            if(i%9==0)printf("\n\n");

    if(solverJedanD(niz)==1)
    {
        for(i=0;i<81;i++)
        {
            if(i%9==0)printf("\n");
            if(niz[i]==0)printf("-");
            printf("%i ",niz[i]);
        }
    }


    return 0;
}
