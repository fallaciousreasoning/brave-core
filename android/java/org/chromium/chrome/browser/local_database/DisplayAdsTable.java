/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.local_database;

public class DisplayAdsTable {
    public static final String TABLE_NAME = "display_ads_table";

    public static final String COLUMN_UUID = "uuid";
    public static final String COLUMN_CREATIVE_INSTANCE_ID = "creative_instance_id";
    public static final String COLUMN_POSITION = "position";
    public static final String COLUMN_TAB_ID = "tab_id";
    public static final String COLUMN_AD_TITLE = "ad_title";
    public static final String COLUMN_AD_DESCRIPTION = "ad_description";
    public static final String COLUMN_AD_CTA_TEXT = "cta_text";
    public static final String COLUMN_AD_CTA_LINK = "cta_link";
    public static final String COLUMN_AD_IMAGE = "ad_image";

    // Create table SQL query
    public static final String CREATE_TABLE = "CREATE TABLE IF NOT EXISTS " + TABLE_NAME
            + "( ID INTEGER PRIMARY KEY AUTOINCREMENT," + COLUMN_UUID + " TEXT,"
            + COLUMN_CREATIVE_INSTANCE_ID + " TEXT," + COLUMN_POSITION + " INT," + COLUMN_TAB_ID
            + " INT," + COLUMN_AD_TITLE + " TEXT," + COLUMN_AD_DESCRIPTION + " TEXT,"
            + COLUMN_AD_CTA_TEXT + " TEXT," + COLUMN_AD_CTA_LINK + " TEXT," + COLUMN_AD_IMAGE
            + " TEXT"
            + ")";

    public DisplayAdsTable() {}

    private String uuid;
    private String creativeInstanceId;
    private int position;
    private int tabId;
    private String adTitle;
    private String adDescription;
    private String adCtaText;
    private String adCtaLink;
    private String adImage;

    public DisplayAdsTable(String creativeInstanceId, int position, int tabId, String adTitle,
            String adCtaText, String adCtaLink, String adImage) {
        this.creativeInstanceId = creativeInstanceId;
        this.position = position;
        this.tabId = tabId;
        this.adTitle = adTitle;
        this.adCtaText = adCtaText;
        this.adCtaLink = adCtaLink;
        this.adImage = adImage;
    }

    public DisplayAdsTable(String uuid, String creativeInstanceId, int position, int tabId,
            String adTitle, String adDescription, String adCtaText, String adCtaLink,
            String adImage) {
        this.uuid = uuid;
        this.creativeInstanceId = creativeInstanceId;
        this.position = position;
        this.tabId = tabId;
        this.adTitle = adTitle;
        this.adDescription = adDescription;
        this.adCtaText = adCtaText;
        this.adCtaLink = adCtaLink;
        this.adImage = adImage;
    }

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getCreativeInstanceId() {
        return creativeInstanceId;
    }

    public void setCreativeInstanceId(String creativeInstanceId) {
        this.creativeInstanceId = creativeInstanceId;
    }

    public int getPosition() {
        return position;
    }

    public void setPosition(int position) {
        this.position = position;
    }

    public int getTabId() {
        return tabId;
    }

    public void setTabId(int tabId) {
        this.tabId = tabId;
    }

    public String getAdTitle() {
        return adTitle;
    }

    public void setAdTitle(String adTitle) {
        this.adTitle = adTitle;
    }

    public String getAdDescription() {
        return adDescription;
    }

    public void setAdDescription(String adDescription) {
        this.adDescription = adDescription;
    }

    public String getAdCtaText() {
        return adCtaText;
    }

    public void setAdCtaText(String adCtaText) {
        this.adCtaText = adCtaText;
    }

    public String getAdCtaLink() {
        return adCtaLink;
    }

    public void setAdCtaLink(String adCtaLink) {
        this.adCtaLink = adCtaLink;
    }

    public String getAdImage() {
        return adImage;
    }

    public void setAdImage(String adImage) {
        this.adImage = adImage;
    }
}
