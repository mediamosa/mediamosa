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
 * The transcode mapping DB class.
 */
class mediamosa_transcode_mapping_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_transcode_mapping';

  // Database fields;
  const ID = 'transcode_mapping_id';
  const TOOL = 'tool';
  const TOOL_LENGTH = 50;
  const NICE_PARAMETER = 'nice_parameter';
  const NICE_PARAMETER_LENGTH = 50;
  const TOOL_PARAMETER = 'tool_parameter';
  const TOOL_PARAMETER_LENGTH = 50;
  const MIN_VALUE = 'min_value';
  const MAX_VALUE = 'max_value';
  const ALLOWED_VALUE = 'allowed_value';
  const ALLOWED_VALUE_LENGTH = 1000;
  const DEFAULT_VALUE = 'default_value';
  const DEFAULT_VALUE_LENGTH = 50;
  const REQUIRED = 'required';
  const REQUIRED_TRUE = 'TRUE';
  const REQUIRED_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
