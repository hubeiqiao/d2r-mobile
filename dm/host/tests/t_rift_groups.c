/* 同生产引擎，注入原生边界；不宣称真机刷怪已验证。
 * Exercise the production engine with injected native boundaries; not device spawn verification. */
#include "../dm_rift_groups.h"
#include "../dm_doorspot.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int dm_target_boss_class(uint32_t cls){return cls==99;}
static struct {dm_rg_candidate c[12];int n,spots,calls,bad,ambiguous;uint32_t id,seen[16];} F;
static int collect(void*v,dm_rg_candidate*out,int cap){(void)v;int n=F.n<cap?F.n:cap;memcpy(out,F.c,(size_t)n*sizeof*out);return n;}
static int spot(void*v,int*x,int*y){(void)v;F.spots++;*x=F.bad?20:32;*y=0;return 1;}
static uint32_t spawn(void*v,uint32_t cls,int x,int y){(void)v;(void)x;(void)y;F.seen[F.calls++]=cls;return F.ambiguous?UINT32_MAX:++F.id;}
static unsigned random_pick(void*v,unsigned n){(void)v;(void)n;return 0;}
static dm_rg_ops ops={NULL,collect,spot,spawn,random_pick};
static void setup(int profile){memset(&F,0,sizeof F);F.id=100;F.n=2;F.c[0]=(dm_rg_candidate){.id=1,.cls=11,.level=7,.hostile=1};F.c[1]=(dm_rg_candidate){.id=2,.cls=12,.level=7,.hostile=1};dm_rift_groups_begin(10,profile,1);dm_rift_groups_step(10,7,0,0,0,0,&ops);}
static void step(uint32_t t){dm_rift_groups_step(10,7,t,20,0,0,&ops);}
int main(void){
 uint64_t u[48]={0},p[16]={0},room[8]={0},ex[32]={0},lv[80]={0};
 *(uint32_t*)u=1;*(uint32_t*)((char*)u+4)=11;*(uint32_t*)((char*)u+8)=501;u[0x38/8]=(uintptr_t)p;p[0x20/8]=(uintptr_t)room;room[0x18/8]=(uintptr_t)ex;ex[0x90/8]=(uintptr_t)lv;*(int*)((char*)lv+0x1f8)=7;
 assert(dm_rift_groups_validate((uintptr_t)u,11,7,(uintptr_t)room));assert(!dm_rift_groups_validate((uintptr_t)u,12,7,(uintptr_t)room));assert(!dm_rift_groups_validate((uintptr_t)u,11,8,(uintptr_t)room));assert(!dm_rift_groups_validate((uintptr_t)u,11,7,123));*(uint32_t*)u=2;assert(!dm_rift_groups_validate((uintptr_t)u,11,7,(uintptr_t)room));*(uint32_t*)u=1;p[0x20/8]=0;assert(!dm_rift_groups_validate((uintptr_t)u,11,7,(uintptr_t)room));

 struct ds_grid g={0};ds_flood(&g);assert(dm_rift_groups_reachable(&g,10,0));assert(!dm_rift_groups_reachable(&g,0,0));assert(!dm_rift_groups_reachable(&g,25,0));assert(!dm_rift_groups_reachable(&g,24,1));assert(!dm_rift_groups_reachable(&g,INT32_MAX,0));g.blk[DS_R*DS_W+DS_R+10]=1;ds_flood(&g);assert(!dm_rift_groups_reachable(&g,10,0));

 setup(0);step(124);assert(!F.calls);dm_rift_groups_step(10,7,125,15,0,0,&ops);assert(!F.calls);step(125);assert(F.calls==2&&F.seen[0]==11&&F.seen[1]==12);assert(dm_rift_groups_alive()==2);step(374);assert(F.calls==2);step(375);assert(F.calls==4&&dm_rift_groups_alive()==4);step(625);assert(F.calls==4);
 dm_rift_groups_death(9,101);assert(dm_rift_groups_alive()==4);dm_rift_groups_death(10,101);dm_rift_groups_death(10,101);assert(dm_rift_groups_alive()==3);dm_rift_groups_death(10,102);step(625);assert(F.calls==6);for(int i=101;i<=106;i++)dm_rift_groups_death(10,(uint32_t)i);step(1000);assert(F.calls==6); /* Three group cap. */
 setup(1);step(125);assert(F.calls==3);step(375);assert(F.calls==3);for(int i=101;i<=103;i++)dm_rift_groups_death(10,(uint32_t)i);step(375);assert(F.calls==6);for(int i=104;i<=106;i++)dm_rift_groups_death(10,(uint32_t)i);step(625);assert(F.calls==6&&dm_rift_groups_created()<=8);
 setup(0);F.n=8;for(int i=2;i<8;i++)F.c[i]=F.c[0];F.c[0].owned=1;F.c[1].boss=1;F.c[2].flags=2;F.c[3].flags=4;F.c[4].flags=8;F.c[5].flags=16;F.c[6].level=8;F.c[7].hostile=0;step(125);assert(!F.calls);
 setup(0);F.n=1;step(125);assert(F.calls==2&&F.seen[0]==11&&F.seen[1]==11);
 setup(0);F.ambiguous=1;step(125);assert(F.calls==1&&dm_rift_groups_disabled());F.ambiguous=0;step(1000);assert(F.calls==1);
 setup(0);F.bad=1;step(125);assert(!F.calls);F.bad=0;step(126);assert(!F.calls);step(375);assert(F.calls==2);
 setup(0);dm_rift_groups_step(10,7,125,20,0,50,&ops);assert(!F.calls);dm_rift_groups_step(11,7,125,20,0,0,&ops);assert(!F.calls);step(125);dm_rift_groups_cancel(10);step(1000);assert(F.calls==2);dm_rift_groups_begin(11,0,1);dm_rift_groups_death(10,101);assert(!dm_rift_groups_alive());
 setup(0);dm_rift_groups_step(10,8,125,20,0,0,&ops);dm_rift_groups_step(10,8,250,40,0,0,&ops);assert(!F.calls); /* Candidates must match current map. */
 puts("rift groups: production policy tests passed (native runtime pending)");return 0;
}
