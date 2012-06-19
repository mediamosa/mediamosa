<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * The database class of asset supplement.
  */

class mediamosa_asset_supplement_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_supplement'; // 1.x name asset_supplement.

  // Database fields;
  const ID = 'supplement_id';
  const ASSET_ID = 'asset_id';
  const SUPPLEMENT = 'supplement';
  const TAG = 'tag';
  const TAG_LENGTH = 255;
  const OAI_PREFIX = 'oai_prefix';
  const OAI_PREFIX_LENGTH = 25;
  const OAI_NAMESPACEURI = 'oai_namespaceuri';
  const OAI_NAMESPACEURI_LENGTH = 255;
  const OAI_SCHEMALOCATION = 'oai_schemalocation';
  const OAI_SCHEMALOCATION_LENGTH = 255;
  const CREATED = 'created';
  const CHANGED = 'changed';
}
