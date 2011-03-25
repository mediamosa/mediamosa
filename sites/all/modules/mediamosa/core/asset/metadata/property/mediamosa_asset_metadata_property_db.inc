<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * The database class of the asset module.
  */

class mediamosa_asset_metadata_property_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_metadata_property';

  // Database fields;
  const ID = 'prop_id';
  const PROPGROUP_ID = 'propgroup_id';
  const APP_ID = 'app_id';
  const NAME = 'prop_name';// 1.x name: name.
  const NAME_LENGTH = 32;
  const TYPE = 'type';
  const TYPE_DATETIME = 'DATETIME';
  const TYPE_INT = 'INT';
  const TYPE_CHAR = 'CHAR';
  const MIN_OCCURENCES = 'min_occurences';
  const MAX_OCCURENCES = 'max_occurences';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const IS_HIDDEN = 'is_hidden';
  const IS_HIDDEN_TRUE = 'TRUE';
  const IS_HIDDEN_FALSE = 'FALSE';
}
