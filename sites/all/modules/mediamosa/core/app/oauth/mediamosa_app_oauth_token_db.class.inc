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
 * App database class object.
 */
class mediamosa_app_oauth_token_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_app_oauth_token';

  // Database fields;
  const TOKEN_KEY = 'token_key';
  const TOKEN_KEY_LENGTH = 255;
  const SECRET = 'secret';
  const SECRET_LENGTH = 255;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const EXPIRES = 'expires';
  const TYPE = 'type';
  const TYPE_LENGTH = 7;
  const TYPE_REQUEST = 'request';
  const TYPE_ACCESS = 'access';
  const APP_ID = 'app_id';
  const APP_ID_NON = 0;
  const CONSUMER_KEY = 'consumer_key';
  const CONSUMER_KEY_LENGTH = 255;
  const AUTHORIZED = 'authorized';
  const AUTHORIZED_NO = 0;
  const AUTHORIZED_YES = 1;
  const CALLBACK_URL = 'callback_url';
  const CALLBACK_URL_LENGTH = 255;
}
