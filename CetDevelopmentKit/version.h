#ifndef VERSION_H
#define VERSION_H

/* 是否为正式版 */
#define IS_RELEASE_VERSION      ( 1 )

/* 图标 */
#define PRODUCT_ICON            "favorite.ico"

/* 版本号定义 */
#define VER_MAJOR               1   /* 主版本号 */
#define VER_MINOR               1   /* 次版本号 */
#define VER_MICRO               2   /* 小版本号 */

/* 版本宏定义 */
#define VERSION_CHECK(major, minor, micro)  ((major<<16)|(minor<<8)|(micro<<0))
#define VERSION_STRING(major, minor, micro) __STR(major) "." __STR(minor) "." __STR(micro)
#define __STR(var)   #var

/* 正式版发布日期 */
#define RELEASE_DATE            "(2024-01-30)"

/* 补丁包号(月+日+序号) */
#define PATCH_PACKET            "b301"

#if (IS_RELEASE_VERSION > 0)
#define TIP_VERSION             RELEASE_DATE
#define APP_VERSION             VERSION_STRING(VER_MAJOR, VER_MINOR, VER_MICRO)
#else
#define TIP_VERSION             RELEASE_DATE " Patch-" PATCH_PACKET
#define APP_VERSION             VERSION_STRING(VER_MAJOR, VER_MINOR, VER_MICRO) "." PATCH_PACKET
#endif

#define APP_NAME                "CetDevelopmentKit"
#define COMPANY_NAME            "CetXiyuan"

/* 文件版本 */
#define FILE_VERSION            VER_MAJOR,VER_MINOR,VER_MICRO
#define FILE_VERSION_STR        APP_VERSION

/* 产品版本 */
#define PRODUCT_VERSION         FILE_VERSION
#define PRODUCT_VERSION_STR     FILE_VERSION_STR

/* 文件描述 */
#define FILE_DESCRIPTION        APP_NAME " based on Qt 5.9.9 (MinGW, 32 bit)"

/* 版权 */
#define LEGAL_COPYRIGHT         "Copyright 2008-2031 The " COMPANY_NAME " Ltd. All rights reserved."

/* 原始应用名 */
#define ORIGINAL_NAME           APP_NAME ".exe"

/* 产品名称 */
#define PRODUCT_NAME            APP_NAME "110"

/* 域名 */
#define ORGANIZATION_DOMAIN     "https://www.cetxiyuan.com/"

#endif // VERSION_H
