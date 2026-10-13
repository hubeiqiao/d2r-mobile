/* 同图刷怪适配只复用 dm_rift.c / dm_unit.h 已使用的偏移和调用：
 * 游戏怪物链 +0x2630 / 下项 +0x158；关卡房间链 +0x10 / 下项 +0x48，
 * 房间载入指针 +0x58、矩形 +0x60..0x6c；碰撞 0x4ce020、空位 0x7160d0、生成 0x7ce630。
 * Same-map spawn adapter reuses offsets/calls already used by dm_rift.c / dm_unit.h:
 * game monster list +0x2630 / next +0x158; level rooms +0x10 / next +0x48,
 * loaded room +0x58, rectangle +0x60..0x6c; collision 0x4ce020, free spot 0x7160d0, spawn 0x7ce630.
 * 原生生成尚待运行验证；默认关闭，服务端单线程调用。 / Native spawning awaits runtime validation; off by default, server-thread only. */
#include "dm_rift_groups.h"
#include "dm_unit.h"
#include "dm_doorspot.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int dm_target_boss_class(uint32_t cls);
extern int dm_rift_server_hostile(uint64_t unit);
/* 仅服务端线程调用。消失不等于死亡；只有权威死亡事件释放并发名额。
 * Server-thread only. Disappearance is not death; only authoritative death releases capacity. */
static struct { int seq, enabled, profile, map, entered, ex, ey, alive, made, groups, stopped, attempted, no_pool_logs; uint32_t entry, last, ids[8]; uint8_t dead[8]; } S;
void dm_rift_groups_begin(int seq, int profile, int enabled) { memset(&S,0,sizeof S); S.seq=seq; S.profile=profile>=0&&profile<4?profile:0; S.enabled=!!enabled; }
void dm_rift_groups_leave(int seq) { if(seq==S.seq) { S.entered=0; S.map=0; } }
void dm_rift_groups_cancel(int seq) { if(seq==S.seq) S.enabled=0; }
int dm_rift_groups_alive(void) { return S.alive; }
int dm_rift_groups_created(void) { return S.made; }
int dm_rift_groups_disabled(void) { return !S.enabled||S.stopped; }
static int own_id(uint32_t id) { for(int i=0;i<S.made;i++) if(S.ids[i]==id) return 1; return 0; }
void dm_rift_groups_death(int seq,uint32_t id) { if(seq!=S.seq) return; for(int i=0;i<S.made;i++) if(S.ids[i]==id&&!S.dead[i]) { S.dead[i]=1; if(S.alive>0) S.alive--; return; } }
void dm_rift_groups_step(int seq,int level,uint32_t tick,int x,int y,uint32_t guardian,const dm_rg_ops *o) {
 if(seq!=S.seq||!S.enabled||S.stopped||guardian||level<=0||!o||!o->collect||!o->spot||!o->spawn||!o->random) return;
 if(!S.entered||S.map!=level) { S.entered=1; S.map=level; S.entry=tick; S.ex=x; S.ey=y; return; }
 if((uint32_t)(tick-S.entry)<125||abs(x-S.ex)+abs(y-S.ey)<16||S.groups>=3||S.made>=8||S.alive>=4||(S.attempted&&(uint32_t)(tick-S.last)<250)) return;
 int count=(S.profile==1||S.profile==3)?3:2; if(count>4-S.alive||count>8-S.made) return;
 S.last=tick; S.attempted=1;
 dm_rg_candidate c[128]; uint32_t cls[8]; int nc=0,n=o->collect(o->ctx,c,128); if(n<0) return; if(n>128)n=128;
 for(int i=0;i<n;i++) { if(!c[i].id||c[i].dead||c[i].level!=level||!c[i].hostile||c[i].owned||c[i].boss||(c[i].flags&30)||own_id(c[i].id)) continue;
  int duplicate=0; for(int j=0;j<nc;j++) duplicate|=cls[j]==c[i].cls; if(!duplicate&&nc<8)cls[nc++]=c[i].cls;
 }
 if(!nc){if(S.no_pool_logs++<3)fprintf(stderr,"[dm] rift-groups seq=%d level=%d no_eligible_native_species\n",S.seq,level);return;}
 /* 每次尝试都消耗冷却，避免墙边持续搜索；至少两种时轮换选种。
  * Every attempt consumes cooldown to avoid repeated wall searches; rotate species when at least two exist. */
 S.last=tick; S.groups++; int before=S.made; unsigned first=o->random(o->ctx,(unsigned)nc)%nc;
 for(int i=0;i<count;i++) { int sx,sy; if(!o->spot(o->ctx,&sx,&sy))break; int d=abs(sx-x)+abs(sy-y); if(d<10||d>24)break;
  uint32_t id=o->spawn(o->ctx,cls[(first+(unsigned)i)%(unsigned)nc],sx,sy);
  if(id==UINT32_MAX|| (id&&own_id(id))) { S.stopped=1; break; }
  if(!id)break;
  S.ids[S.made++]=id; S.alive++;
 }
 fprintf(stderr,"[dm] rift-groups seq=%d level=%d group=%d native_species=%d created=%d alive=%d total=%d ambiguous=%d\n",S.seq,level,S.groups,nc,S.made-before,S.alive,S.made,S.stopped);
}
int dm_rift_groups_reachable(const struct ds_grid *g,int dx,int dy){ if(dx < -DS_R || dx > DS_R || dy < -DS_R || dy > DS_R)return 0;int d=abs(dx)+abs(dy);return d>=10&&d<=24&&ds_free(g,dx+DS_R,dy+DS_R)&&ds_dist(g,dx+DS_R,dy+DS_R)>=0;}
static uint64_t rd(uint64_t a){return a?*(volatile uint64_t *)(uintptr_t)a:0;}
static uint32_t u32(uint64_t a){return *(volatile uint32_t *)(uintptr_t)a;}
int dm_rift_groups_validate(uint64_t u,uint32_t cls,int level,uint64_t room){if(!u||!room)return 0;uint64_t path=rd(u+0x38);return u32(u)==1&&u32(u+4)==cls&&u32(u+8)!=0&&dm_unit_level(u)==level&&path&&rd(path+0x20)==room;}
enum { RG_ROOM_WALK_MAX=2000, RG_RECTS_MAX=64, RG_SPOT_TRIES=16 };
typedef struct {uint8_t *B;uint64_t G,p,lv;int level,x,y,grid_ready,rooms_ready,spot_tries;uint32_t mask;struct ds_grid grid;uint64_t rooms[DS_N];} native_ctx;
/* 单次分组只遍历一次原生房间链，栅格缓存随后查询 O(1)。范围外视为封闭。
 * Walk the native room chain once per group, then use an O(1) raster cache; outside is closed. */
static int cache_rooms(native_ctx*c){if(c->rooms_ready)return c->rooms_ready>0;int walk=0,rects=0;uint64_t r=dm_level_rooms(c->lv);
 for(;r&&walk<RG_ROOM_WALK_MAX;r=rd(r+0x48),walk++) {uint64_t loaded=rd(r+0x58);if(!loaded)continue;
 int64_t x=(int64_t)*(int32_t*)(uintptr_t)(r+0x60)*5,y=(int64_t)*(int32_t*)(uintptr_t)(r+0x64)*5,w=(int64_t)*(int32_t*)(uintptr_t)(r+0x68)*5,h=(int64_t)*(int32_t*)(uintptr_t)(r+0x6c)*5;
 int64_t left=x-(c->x-DS_R),top=y-(c->y-DS_R),right=left+w,bottom=top+h;if(w<=0||h<=0||right<=0||bottom<=0||left>=DS_W||top>=DS_W)continue;
 if(++rects>RG_RECTS_MAX){c->rooms_ready=-1;return 0;}if(left<0)left=0;if(top<0)top=0;if(right>DS_W)right=DS_W;if(bottom>DS_W)bottom=DS_W;
 for(int iy=(int)top;iy<(int)bottom;iy++)for(int ix=(int)left;ix<(int)right;ix++)if(!c->rooms[iy*DS_W+ix])c->rooms[iy*DS_W+ix]=loaded;
 }if(r){c->rooms_ready=-1;return 0;}c->rooms_ready=1;return 1;}
static uint64_t room_at(native_ctx*c,int x,int y){int64_t ix=(int64_t)x-c->x+DS_R,iy=(int64_t)y-c->y+DS_R;return ix<0||ix>=DS_W||iy<0||iy>=DS_W?0:c->rooms[(int)iy*DS_W+(int)ix];}
static int collect(void *v,dm_rg_candidate *out,int cap) {native_ctx *c=v;int n=0,walk=0;for(int b=0;b<128&&n<cap;b++)for(uint64_t u=rd(c->G+0x2630+8*b);u&&walk++<4096;u=rd(u+0x158)){
 if(u32(u)!=1||dm_unit_level(u)!=c->level)continue;
 uint64_t path=rd(u+0x38),data=rd(u+0x10);if(!path||!rd(path+0x20)||!data)continue;
 uint32_t cls=u32(u+4);out[n++]=(dm_rg_candidate){.id=u32(u+8),.cls=cls,.level=c->level,.hostile=dm_rift_server_hostile(u),.owned=!!(u32(u+0x124)&0x80000000u),.boss=dm_target_boss_class(cls),.flags=*(uint8_t *)(uintptr_t)(data+0x1a),.dead=u32(u+0xc)==0||u32(u+0xc)==12};
 }return n;}
static int spot(void *v,int *x,int *y){native_ctx*c=v;
 if(!cache_rooms(c))return 0;
 if(!c->grid_ready){for(int iy=0;iy<DS_W;iy++)for(int ix=0;ix<DS_W;ix++){int gx=c->x+ix-DS_R,gy=c->y+iy-DS_R;uint64_t r=room_at(c,gx,gy);uint32_t f=r?((uint32_t(*)(uint64_t,int,int,int))(uintptr_t)(c->B+0x4ce020))(r,gx,gy,(int)c->mask)&0xffff:1;if(ix==DS_R&&iy==DS_R)f&=~0x1000u;c->grid.blk[iy*DS_W+ix]=f!=0;}ds_flood(&c->grid);c->grid_ready=1;}
 /* 无脚下兜底。原版空位调整后重新验证可达性、距离和当前图房间。
  * No feet fallback. Revalidate reachability, distance and same-map room after native adjustment. */
 int start=(int)arc4random_uniform(DS_N);for(int k=0;k<DS_N&&c->spot_tries<RG_SPOT_TRIES;k++){int at=(start+k)%DS_N,dx=at%DS_W-DS_R,dy=at/DS_W-DS_R,d=abs(dx)+abs(dy);if(d<10||d>24||ds_dist(&c->grid,dx+DS_R,dy+DS_R)<0)continue;
 int32_t p[2]={c->x+dx,c->y+dy};uint64_t r=room_at(c,p[0],p[1]),out=0;if(!r)continue;c->spot_tries++;((void(*)(uint64_t,uint64_t,int32_t*,int,int,uint64_t*,int,int))(uintptr_t)(c->B+0x7160d0))(c->G,r,p,3,0x400,&out,4,100);
 dx=p[0]-c->x;dy=p[1]-c->y;if(!out||room_at(c,p[0],p[1])!=out||!dm_rift_groups_reachable(&c->grid,dx,dy))continue;
 if(((uint32_t(*)(uint64_t,int,int,int))(uintptr_t)(c->B+0x4ce020))(out,p[0],p[1],(int)c->mask)&0xffff)continue;
 *x=p[0];*y=p[1];return 1;}return 0;}
static uint32_t spawn(void*v,uint32_t cls,int x,int y){native_ctx*c=v;uint64_t r=room_at(c,x,y);if(!r)return 0;uint64_t u=((uint64_t(*)(uint64_t,uint64_t,int,int,int,int,int,int,uint64_t))(uintptr_t)(c->B+0x7ce630))(c->G,r,1,(int)cls,x,y,1,0,0x10);if(!u)return UINT32_MAX;if(!dm_rift_groups_validate(u,cls,c->level,r))return UINT32_MAX;uint32_t id=u32(u+8);return id?id:UINT32_MAX;}
static unsigned random_pick(void*v,unsigned n){(void)v;return arc4random_uniform(n);}
void dm_rift_groups_tick(uint8_t*B,uint64_t G,uint64_t player,int level,uint32_t tick,uint32_t guardian){if(!S.enabled||S.stopped||guardian||!B||!G||!player||dm_unit_level(player)!=level)return;uint32_t pm=u32(player+0xc);if(pm==0||pm==17)return;uint64_t p=rd(player+0x38),lv=dm_unit_level_ptr(player);if(!p||!lv||!rd(p+0x20))return;native_ctx c={.B=B,.G=G,.p=player,.lv=lv,.level=level,.x=*(uint16_t*)(uintptr_t)(p+2),.y=*(uint16_t*)(uintptr_t)(p+6),.mask=u32(p+0x64)};if(!c.mask)c.mask=0x1c09;dm_rg_ops o={&c,collect,spot,spawn,random_pick};dm_rift_groups_step(S.seq,level,tick,c.x,c.y,guardian,&o);}
