<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Example settings file for download interface.
 */

// Load default.
require_once getcwd() . '/sites/default/settings.php';

// Set my settings;
/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * APP REST interface. Putting it on FALSE will disable the interface
 * for this URL / Location, disabling all REST calls; except upload/download
 * REST calls, which is controlled by mediamosa_app_upload.
 */
$conf['mediamosa_app'] = FALSE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * APP REST upload interface. The upload setting allows REST call
 * relating to uploading files, it will not allow other REST calls
 * unless 'mediamosa_app' is TRUE also. To setup this interface
 * as an upload interface, put 'mediamosa_app' to FALSE and set
 * 'mediamosa_app_upload' to TRUE.
 */
$conf['mediamosa_app_upload'] = FALSE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * APP REST download interface. The download setting allows you
 * to download mediafile using tickets. For now its used to download
 * files and still images.
 * Warning: If your download servers point to the admin, then make sure
 * sure you allow this setting on the admin, else your MediaMosa status page
 * will show failures.
 */
$conf['mediamosa_app_download'] = TRUE;

/**
 * Default setting, TRUE / FALSE for enabling / disabling the
 * CMS admin interface. You can turn on the admin with app interface
 * but remember that some url like /user conflicts between the CMS and
 * app interface.
 */
$conf['mediamosa_admin'] = FALSE;

/**
 * The 'mediamosa_installation_id' defines the default install ID for multiple
 * installations of mediamosa. For now only job servers need to have unique IDs.
 * Best practise is to give each MediaMosa installation its own ID. F.e. if you
 * have 'job1.mediamosa.example' for your 1st job server, then specify 'job1' as
 * installation ID here. 'admin.mediamosa.example' would be 'admin' as
 * installation ID, etc, etc. Max length is 16 chars.
 */
$conf['mediamosa_installation_id'] = 'download';
