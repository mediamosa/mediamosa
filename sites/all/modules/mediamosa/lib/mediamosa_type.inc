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
 * The mediamosa TYPE class.
 */

class mediamosa_type {

  // ------------------------------------------------------------------ Consts.
  // Store NOW() time as date
  const FUNC_DATETIME_NOW                = 'NOW()';

  // Default fixed values.
  const ORDER_DIRECTION_ASC = 'asc';
  const ORDER_DIRECTION_DESC = 'desc';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Check the input based on type.
   * Will throw exception if not met with conditions.
   * Note: You must accept the value returned as the new value for this param.
   *
   * @param string $param
   *  name of param.
   * @param string $param_type
   *  Type of the param, see TYPE_* list.
   * @param mixed $mixed_param_value
   *  Either string or array (with strings)
   * @param boolean $is_required
   *  Supplied value must be provided
   * @param boolean $toplevel
   *  Internal boolean for array loops, leave as is
   * @return new param value
   */
  static public function check($param, $param_type, $mixed_param_value, $is_required = FALSE, $range_start = NULL, $range_end = NULL, $allowed_values = NULL, $toplevel = TRUE) {

    // If its an array, then walk through the array and check item for item by recalling ourselfs.
    if (is_array($mixed_param_value)) {
      // If the top array is empty and required, then throw an error.
      // Any child array is allowed to be empty.
      if (empty($mixed_param_value) && $is_required && $toplevel) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => $param, '@type' => $param_type));
      }

      foreach ($mixed_param_value as $key => $value) {
        $mixed_param_value[$key] = self::check($param, $param_type, $value, $is_required, $range_start, $range_end, $allowed_values, FALSE);
      }

      return $mixed_param_value; // done.
    }

    // When required and isnull/empty string, then in any type its an error
    if ($is_required && (is_null($mixed_param_value) || trim($mixed_param_value) == '')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => $param, '@type' => $param_type));
    }

    // If NULL, then value is not set.
    if (is_null($mixed_param_value)) {
      return NULL;
    }

    // Now based on type, check contents when needed.
    switch ($param_type) {
      case mediamosa_sdk::TYPE_LANGUAGE_CODE:
        // ISO 639-1.
        $a_language_codes = explode(',', 'aa,ab,ae,af,ak,am,an,ar,as,av,ay,az,ba,be,bg,bh,bi,bm,bn,bo,br,bs,ca,ce,ch,co,cr,cs,cu,cv,cy,da,de,dv,dz,ee,el,en,eo,es,et,eu,fa,ff,fi,fj,fo,fr,fy,ga,gd,gl,gn,gu,gv,ha,he,hi,ho,hr,ht,hu,hy,hz,ia,id,ie,ig,ii,ik,io,is,it,iu,ja,jv,ka,kg,ki,kj,kk,kl,km,kn,ko,kr,ks,ku,kv,kw,ky,la,lb,lg,li,ln,lo,lt,lu,lv,mg,mh,mi,mk,ml,mn,mo,mr,ms,mt,my,na,nb,nd,ne,ng,nl,nn,no,nr,nv,ny,oc,oj,om,or,os,pa,pi,pl,ps,pt,qu,rm,rn,ro,ru,rw,ry,sa,sc,sd,se,sg,sh,si,sk,sl,sm,sn,so,sq,sr,ss,st,su,sv,sw,ta,te,tg,th,ti,tk,tl,tn,to,tr,ts,tt,tw,ty,ug,uk,ur,uz,ve,vi,vo,wa,wo,xh,yi,yo,za,zh,zu');

        if (!in_array(mediamosa_unicode::strtolower($mixed_param_value), $a_language_codes)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        // To lower.
        $checked_param_value = (string)mediamosa_unicode::strtolower($mixed_param_value);
        break;

      case mediamosa_sdk::TYPE_APP_ID:
        if ( (!is_int($mixed_param_value) && !ctype_digit($mixed_param_value))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_APP_ID, array('@app_id' => $mixed_param_value));
        }

        $checked_param_value = (int)$mixed_param_value;

        // If required, then value must be > 0, if not required, then we allow >= 0.
        if ($checked_param_value < 0) { // unsigned only
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        break;

      case mediamosa_sdk::TYPE_SERIAL:
        // Same as UINT, but when required the value must be > 0. When not required, the value must be >= 0.
        if (!is_numeric($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (int)$mixed_param_value;

        // If required, then value must be > 0, if not required, then we allow >= 0.
        if ( ($is_required && $checked_param_value < 1) || (!$is_required && $checked_param_value < 0) ) { // unsigned only
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        break;

      case mediamosa_sdk::TYPE_JOB:
        $a_job_types = array(
          mediamosa_job_db::JOB_TYPE_ANALYSE,
          mediamosa_job_db::JOB_TYPE_DELETE_MEDIAFILE,
          mediamosa_job_db::JOB_TYPE_STILL,
          mediamosa_job_db::JOB_TYPE_TRANSCODE,
          mediamosa_job_db::JOB_TYPE_RETRANSCODE,
          mediamosa_job_db::JOB_TYPE_UPLOAD
        );

        if (!in_array(mediamosa_unicode::strtoupper($mixed_param_value), $a_job_types)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNKNOWN_JOB_TYPE);
        }

        $checked_param_value = (string)mediamosa_unicode::strtoupper($mixed_param_value);
        break;

      case mediamosa_sdk::TYPE_JOB_STATUS:
        $a_job_statuses = array(
          mediamosa_job_db::JOB_STATUS_WAITING,
          mediamosa_job_db::JOB_STATUS_INPROGRESS,
          mediamosa_job_db::JOB_STATUS_FINISHED,
          mediamosa_job_db::JOB_STATUS_FAILED,
          mediamosa_job_db::JOB_STATUS_CANCELLED
        );

        if (!in_array(mediamosa_unicode::strtoupper($mixed_param_value), $a_job_statuses)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNKNOWN_JOB_STATUS);
        }

        $checked_param_value = (string)mediamosa_unicode::strtoupper($mixed_param_value);
        break;

      case mediamosa_sdk::TYPE_JOB_PROGRESS:
        // 0.000 to 1.000.
        if (
          (!is_float($mixed_param_value) || (float)$mixed_param_value < 0 || (float)$mixed_param_value > 1 || mediamosa_unicode::strlen($mixed_param_value) > 5) &&
          $mixed_param_value != ''
        ) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNKNOWN_JOB_PROGRESS);
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_LIMIT:
        $range_start = 0;
        if ($range_end < 1) {
          $range_end = mediamosa_settings::LIMIT_MAX;
        }

      case mediamosa_sdk::TYPE_JOB_ID:
      case mediamosa_sdk::TYPE_UINT:
      case mediamosa_sdk::TYPE_EOR:
        if (!is_numeric($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        $checked_param_value = (int)$mixed_param_value;
        if ($checked_param_value < 0) { // unsigned only
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
        break;

      case mediamosa_sdk::TYPE_INT:
        if (!is_numeric($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (int)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_COMMAND:
        // Eg. "-b", "-ar"
        $mapping_name = "[-a-zA-Z_]+";

        // Eg. "176", "black", "<switch>"
        $mapping_sub_value = "[a-zA-Z0-9_<>]+";

        // Eg. "pad=176:144:0:6:black", "crop=23:34:64:62"
        $mapping_sub_line = "([a-zA-Z0-9_<>]+=$mapping_sub_value(:$mapping_sub_value+)*)";

        // Eg. "200000", "22050", "176x132", "'pad=176:144:0:6:black,crop=0:4:1280:540'"
        $mapping_value = "([a-zA-Z0-9._<>]+|(\'$mapping_sub_line+(,$mapping_sub_line)*\')+)";

        // Eg. "-b:200000;-ar:22050;-s:176x132;-vf:'pad=176:144:0:6:black,crop=0:4:1280:540,scale=1280:656'"
        if (preg_match("/^(($mapping_name:$mapping_value){0,1}|$mapping_name:$mapping_value(;$mapping_name:$mapping_value)*)$/", $mixed_param_value) !== 1) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_ALPHA:
        if (!ctype_alpha($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      // Check ticket id for download/still downloads.
      case mediamosa_sdk::TYPE_TICKET_ID:
        if (preg_match("@^([a-zA-Z0-9]+)(\\.asx)?$@", $mixed_param_value, $matches) !== 1) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_ALPHA_NUM:
        if (!ctype_alnum($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }
      // FIXME: MediaMosa 1.x doesnt check the input here...
      case mediamosa_sdk::TYPE_SUPPLEMENT_ID:
      case mediamosa_sdk::TYPE_USER_ID:
      case mediamosa_sdk::TYPE_GROUP_ID:
      case mediamosa_sdk::TYPE_ASSET_ID:
      case mediamosa_sdk::TYPE_MEDIAFILE_ID:
      case mediamosa_sdk::TYPE_COLLECTION_ID:
      case mediamosa_sdk::TYPE_BATCH_ID:
        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_PRINTABLE:
        // Must contain printable chars
        if (!mediamosa_unicode::printable($mixed_param_value) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE_TAG:
        if ($mixed_param_value != '' && !ctype_alpha(mediamosa_unicode::substr($mixed_param_value, 0, 1))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALUE_MUST_START_WITH_ALPHABETIC_CHAR, array('@param' => $param, '@type' => $param_type));
        }

      case mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE:
        if (!ctype_alnum(str_replace('_', '', $mixed_param_value)) && $mixed_param_value != '') {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_CQL_ASSET:
      case mediamosa_sdk::TYPE_CQL_COLLECTION:
      case mediamosa_sdk::TYPE_CQL_JOB:
      case mediamosa_sdk::TYPE_STRING:
        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_XML_OAI:
        // FIXME: dont like the 1.x way we prefix the XML declaration to the XML... XML declaration is required in XML....
        try {
          set_error_handler('mask_xml_errors');

          $o_dom_document = new DOMDocument();
          if (!$o_dom_document->loadXML('<?xml version="1.0"?>' . $mixed_param_value)) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
          }

          restore_error_handler();
        }
        catch (Exception $e) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_SUPPLEMENT);
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_XML_VALIDATED:
        // Validate xml if value is required or not empty.
        if ($is_required || $mixed_param_value != '') {
          try {
            if (!simplexml_load_string($mixed_param_value)) {
              throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
            }
          }
          catch (Exception $e) {
            assert($e);
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
          }
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_FLOAT:
        if (!is_numeric($mixed_param_value) || (!is_int($mixed_param_value) && !is_float($mixed_param_value))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = floatval($mixed_param_value);
        break;

      case mediamosa_sdk::TYPE_SEARCH_MATCH:
      case mediamosa_sdk::TYPE_SEARCH_STRING:
      case mediamosa_sdk::TYPE_SEARCH_INT:
      case mediamosa_sdk::TYPE_SEARCH_DATETIME:
      case mediamosa_sdk::TYPE_SEARCH_BOOL:
        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_MIMETYPE:
        // @todo: some check here.
        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_BOOL_IS_SLAVED:
      case mediamosa_sdk::TYPE_BOOL:
        if (!is_bool($mixed_param_value)) {
          if (!in_array(mediamosa_unicode::strtolower($mixed_param_value), array('true', 'false', ''))) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
          }

          $mixed_param_value = (mediamosa_unicode::strtolower($mixed_param_value) == 'true') ? TRUE : FALSE;
        }

        $checked_param_value = (bool)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_DATETIME:
      case mediamosa_sdk::TYPE_DATETIME_UTC:
        if ($mixed_param_value == '') {
          return '';
        }

        // If we supply NOW(), we fill in the time as UTC.
        if (mediamosa_unicode::strtoupper($mixed_param_value) == self::FUNC_DATETIME_NOW) {
          $mixed_param_value = mediamosa_datetime::utc_current_timestamp_now();
        }

        // Fix, test etc.
        $checked_param_value = mediamosa_datetime::date8601_to_timestamp($mixed_param_value, $param, $param_type);
        break;

      case mediamosa_sdk::TYPE_ORDER_DIRECTION:
        if (!in_array(mediamosa_unicode::strtolower($mixed_param_value), array(self::ORDER_DIRECTION_ASC, self::ORDER_DIRECTION_DESC))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = mediamosa_unicode::strtolower($mixed_param_value);
        break;


      case mediamosa_sdk::TYPE_URI:
        // @todo: no check for URI type.
        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_URL:
        if (parse_url($mixed_param_value) === FALSE) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_URL_URI:
        // FIXME: need some checker here.
        $checked_param_value = (string)$mixed_param_value;
        break;


      case mediamosa_sdk::TYPE_AUT_GROUP_TYPE:
        $mixed_param_value = mediamosa_unicode::strtoupper($mixed_param_value);

        if (!in_array($mixed_param_value, array(mediamosa_aut_group_db::AUT_GROUP_TYPE_DOMAIN, mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_DELETE:
        if (!in_array($mixed_param_value, array('cascade'))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_DOMAIN:
        if ($mixed_param_value != '' && !mediamosa_aut::valid_domain($mixed_param_value)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_REALM:
        if ($mixed_param_value != '' && !mediamosa_aut::valid_realm($mixed_param_value)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_REALM:
        if (!mediamosa_aut::is_group_realm($mixed_param_value)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_DOMAIN:
        if (!mediamosa_aut::is_group_domain($mixed_param_value)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_OPERATOR:
        if (!in_array(mediamosa_unicode::strtoupper($mixed_param_value), array('OR', 'AND'))) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      case mediamosa_sdk::TYPE_FILENAME:
        if (strpos($mixed_param_value, "/") !== FALSE) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
        }

        $checked_param_value = (string)$mixed_param_value;
        break;

      default:
        throw new mediamosa_exception_program_error(strtr('Using unspecified type (@type) in type lib used for param @param.', array('@param' => $param, '@type' => $param_type)));
    }

    if (is_bool($checked_param_value)) {
      assert(!isset($allowed_values)); // do not specify when bool.
      return $checked_param_value;
    }

    if (!isset($checked_param_value) || (!is_string($checked_param_value) && !is_int($checked_param_value))) {
      throw new mediamosa_exception_program_error('Value type param @param (@type) is not integer or string value.', array('@param' => $param, '@type' => $param_type));
    }

    // Check if value is allowed as value (if specified and if required var).
    if (isset($allowed_values) && !in_array($checked_param_value, $allowed_values)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_VALUE_NOT_ALLOWED, array('@param' => $param, '@type' => $param_type, '@allowed_values' => implode("', '", $allowed_values)));
    }

    // Now do a range check.
    return is_string($checked_param_value) ? self::string_range($param, $checked_param_value, $range_start, $range_end) : self::int_range($param, $checked_param_value, $range_start, $range_end);
  }

  /**
   * Make sure the integer is in the valid range.
   *
   * @param string $param
   *  The Param name.
   * @param integer $int_val
   *  The value.
   * @param integer $min_value
   *  The minimum value allowed.
   * @param unknown_type $max_value
   *  The maximum value allowed.
   * @return
   *  Returns the same value, use it for future changes.
   *
   * Considered success when returning.
   * Returns the same value, use it for future changes.
   */
  static private function int_range($param, $int_val, $min_value, $max_value) {
    // Must be int type.
    assert(is_int($int_val));

    if (is_null($min_value) && is_null($max_value)) {
      return $int_val;
    }

    // To small?
    if (!is_null($min_value) && $int_val < $min_value) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_INT_TO_SMALL, array('@param' => $param, '@min_value' => $min_value));
    }

    // To big?
    if (!is_null($max_value) && $int_val > $max_value) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_INT_TO_BIG, array('@param' => $param, '@max_value' => $max_value));
    }

    return $int_val;
  }

  /**
   * Make sure that the string is nog to long or to short.
   *
   * @param string $param
   *  The Param name.
   * @param string $string_val
   *  The string value.
   * @param integer $min_length
   *  The minimum length.
   * @param integer $max_length
   *  The maximum length.
   * @return
   *  Returns the same value, use it for future changes.
   */
  static private function string_range($param, $string_val, $min_length, $max_length) {
    // Must be string type.
    assert(is_string($string_val));

    if (is_null($min_length) && is_null($max_length)) {
      return $string_val;
    }

    // To short?
    if (!is_null($min_length) && mediamosa_unicode::strlen($string_val) < $min_length) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_STRING_TO_SHORT, array('@param' => $param, '@min_value' => $min_length));
    }

    // To long?
    if (!is_null($max_length) && mediamosa_unicode::strlen($string_val) > $max_length) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_STRING_TO_LONG, array('@param' => $param, '@max_length' => $max_length));
    }

    return $string_val;
  }
}

/**
 * Mask XML errors.
 */
function mask_xml_errors() {
}
