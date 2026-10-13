// 真实 UIKit 离屏测试，采用生产 HUD 与本地化表；不创建窗口或游戏实例。
// Actual UIKit offscreen test using production HUD/localization; creates no window/game instance.
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../host/dm_i18n_tab.h"
void fixture_setup(int tier, int inc);
void dm_rift_hud_draw(UIView *host, CGRect rect, BOOL hide);
static int zh;
// 语言由测试选择，文字直接来自生产表。 | Test picks language; text comes directly from production table.
const char *dm_ui(const char *key) {
    for (unsigned i = 0; i < sizeof k_dm_i18n / sizeof k_dm_i18n[0]; i++)
        if (!strcmp(key, k_dm_i18n[i].k)) return zh ? k_dm_i18n[i].zh : k_dm_i18n[i].en;
    return key;
}
static void collect_labels(CALayer *layer, NSMutableArray<NSString *> *labels) {
    if ([layer isKindOfClass:CATextLayer.class] && [(CATextLayer *)layer string])
        [labels addObject:[(CATextLayer *)layer string]];
    for (CALayer *child in layer.sublayers) collect_labels(child, labels);
}
int main(void) {
    @autoreleasepool {
        const char *output = getenv("RIFT_HUD_OUTPUT");
        if (!output || output[0] != '/') { fprintf(stderr, "Set absolute RIFT_HUD_OUTPUT\n"); return 2; }
        NSString *directory = @(output);
        UIView *view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 1194, 834)];
        view.backgroundColor = [UIColor colorWithRed:.12 green:.15 blue:.18 alpha:1];
        const int cases[][3] = {{100,3,100}, {99,3,100}, {74,3,77}, {100,0,100}};
        int checks = 0, fails = 0; NSMutableString *report = [NSMutableString string];
        for (zh = 0; zh < 2; zh++) for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++) {
            fixture_setup(cases[i][0], cases[i][1]);
            dm_rift_hud_draw(view, view.bounds, NO); [CATransaction flush];
            NSMutableArray<NSString *> *labels = [NSMutableArray array]; collect_labels(view.layer, labels);
            NSString *wanted = cases[i][1] ? [NSString stringWithFormat:@(dm_ui("dmRiftHudNext")), cases[i][2]] : @(dm_ui("dmRiftHudSame"));
            checks++; if (![labels containsObject:wanted]) fails++;
            for (NSString *label in labels)
                if ([label containsString:@"103"] || [label containsString:@"102"] || [label containsString:@"101"]) fails++;
            [report appendFormat:@"lang=%@ tier=%d inc=%d labels=%@ expected=%@\n", zh ? @"zh" : @"en", cases[i][0], cases[i][1], labels, wanted];
            // 直接绘制真实 CALayer 树，无截图替代品。 | Draw the actual CALayer tree, not a mocked screenshot.
            UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:view.bounds.size];
            UIImage *image = [renderer imageWithActions:^(UIGraphicsImageRendererContext *ctx) { [view.layer renderInContext:ctx.CGContext]; }];
            NSString *filename = [NSString stringWithFormat:@"hud-%@-%d-%d.png", zh ? @"zh" : @"en", cases[i][0], cases[i][1]];
            if (![UIImagePNGRepresentation(image) writeToFile:[directory stringByAppendingPathComponent:filename] atomically:YES]) fails++;
        }
        [report appendFormat:@"Actual production dm_rift.c + actual UIKit dm_rift_hud.m: %d checks, %d failures\n", checks, fails];
        if (![report writeToFile:[directory stringByAppendingPathComponent:@"results.txt"] atomically:YES encoding:NSUTF8StringEncoding error:NULL]) { fprintf(stderr, "Cannot save results\n"); return 2; }
        fprintf(stdout, "%s", report.UTF8String); return fails ? 1 : 0;
    }
}
