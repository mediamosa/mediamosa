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

class mediamosa_asset_metadata_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_metadata';

  // Database fields;
  const ID = 'metadata_id'; // 1.x: id
  const ASSET_ID = 'asset_id';
  const PROP_ID = 'prop_id';
  const VAL_CHAR = 'val_char';
  const VAL_CHAR_LFT = 'val_char_lft'; // new.
  const VAL_CHAR_LFT_LENGTH = 50;
  const VAL_CHAR_RGHT = 'val_char_rght'; // new.
  const VAL_CHAR_RGHT_LENGTH = 50;
  const VAL_DATETIME = 'val_datetime';
  const VAL_INT = 'val_int';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
