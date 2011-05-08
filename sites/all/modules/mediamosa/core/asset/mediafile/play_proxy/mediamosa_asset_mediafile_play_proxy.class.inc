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
  * Play Proxy functions
  */

class mediamosa_asset_mediafile_play_proxy {
  // ------------------------------------------------------------------ Consts.
  /**
   * Response types
   */
  const RESPONSE_URI = 'plain';
  const RESPONSE_STILL = 'still';
  const RESPONSE_METAFILE = 'metafile';
  const RESPONSE_OBJECT = 'object';
  const RESPONSE_DOWNLOAD = 'download';

  // Params in URI replacements.
  const PARAM_URI_TICKET = '{TICKET}';
  const PARAM_URI_SCRIPT = '{SCRIPT}';

  // Default width & height for playproxy flash object HTML.
  CONST PP_FLASH_OBJECT_HEIGHT = 240;
  CONST PP_FLASH_OBJECT_WIDTH = 320;

  /**
   *  Container types
   */
  const CONTAINER_TYPE_WMV = 'asf';
  const CONTAINER_TYPE_MP3 = 'mp3';
  const CONTAINER_TYPE_MP4 = 'mov;mp4;m4a;3gp;3g2;mj2';
  const CONTAINER_TYPE_FLV = 'flv';
  const CONTAINER_TYPE_OGG = 'ogg';

  /**
   * Video codec types.
   */
  const VIDEO_CODEC_H264 = 'h264';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get the uri of the correct media server.
   *
   * @param string $response_type
   * @param array $mediafile_ext
   */
  static public function get_media_server($response_type, array $mediafile_ext = array()) {

    switch ($response_type) {
      case self::RESPONSE_STILL:
        return mediamosa_db::db_query(
          "SELECT REPLACE(#server_path, :base_uri, #server_uri) FROM {#mediamosa_server} WHERE #server_status = 'ON' AND #server_type = :server_type ORDER BY rand() LIMIT 1",
          array(
            '#server_path' => mediamosa_server_db::SERVER_PATH,
            '#server_uri' => mediamosa_server_db::SERVER_URI,
            '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
            '#server_status' => mediamosa_server_db::SERVER_STATUS,
            '#server_type' => mediamosa_server_db::SERVER_TYPE,
            ':base_uri' => mediamosa_server_db::SERVER_PATH_BASE_URI,
            ':server_type' => mediamosa_server_db::SERVER_TYPE_STILL,
          )
        )->fetchField();

      case self::RESPONSE_DOWNLOAD:
        return mediamosa_db::db_query(
          "SELECT REPLACE(#server_path, :base_uri, #server_uri) FROM {#mediamosa_server} WHERE #server_status = 'ON' AND #server_type = :server_type ORDER BY rand() LIMIT 1",
          array(
            '#server_path' => mediamosa_server_db::SERVER_PATH,
            '#server_uri' => mediamosa_server_db::SERVER_URI,
            '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
            '#server_status' => mediamosa_server_db::SERVER_STATUS,
            '#server_type' => mediamosa_server_db::SERVER_TYPE,
            ':base_uri' => mediamosa_server_db::SERVER_PATH_BASE_URI,
            ':server_type' => mediamosa_server_db::SERVER_TYPE_DOWNLOAD,
          )
        )->fetchField();
    }

    // Needed at this point.
    assert(!empty($mediafile_ext));

    // Get the URI.
    $container_type = (empty($mediafile_ext['metadata']['container_type']) ? NULL : $mediafile_ext['metadata']['container_type']);
    $video_codec = (empty($mediafile_ext['metadata']['video_codec']) ? NULL : $mediafile_ext['metadata']['video_codec']);
    $mime_type = (empty($mediafile_ext['metadata']['mime_type']) ? NULL : $mediafile_ext['metadata']['mime_type']);
    $uri = mediamosa_server::get_uri_streaming($container_type, $video_codec, $mime_type);

    if (!$uri) { // No streaming profile found
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STREAMING_PROFILE_NOT_FOUND, array('@video_container' => $container_type));
    }

    return $uri;
  }

  /**
   * This function converts miliseconds to the format specified
   *
   * @param integer $msec
   * @param string $container_type
   *  See CONTAINER_TYPE_*
   *
   * @return
   *  requested time format.
   */
  function convert_msec($input, $container_type) {
    $output = FALSE;

    // bereken uur, minuut en seconde
    $second = 1000;
    $minute = $second * 60;
    $hour = $minute * 60;
    $a_output['hours'] = floor($input / $hour);
    $input -= $a_output['hours'] * $hour;
    $a_output['minutes'] = floor($input / $minute);
    $input -= $a_output['minutes'] * $minute;
    $a_output['seconds'] = floor($input / $second);
    $input -= $a_output['seconds'] * $second;
    $msec = $input;

    switch ($container_type) {
      case self::CONTAINER_TYPE_WMV: // hh:mm:ss.fract(=sec/100)
        $fraction = mediamosa_unicode::substr($msec, 0, 2);
        foreach ($a_output as $subject => $value) {
          if ($a_output[$subject] < 10) {
            $a_output[$subject] = '0' . $a_output[$subject];
          }
        }
        $output = implode(':', $a_output) . '.' . $fraction;
        break;

      case self::CONTAINER_TYPE_MP4: // hh:mm:ss:frames(=sec/30).
      default:
        $frame = 1 / 30;
        $fraction = round($msec / $frame / 1000, 1);
        if ($fraction == 30) {
          $fraction = 0;
          $a_output['seconds']++;
        }

        foreach ($a_output as $subject => $value) {
          if ($a_output[$subject] < 10) {
            $a_output[$subject] = '0' . $a_output[$subject];
          }
        }
        $output = implode(':', $a_output) . ':' . $fraction;
        break;
    }

    return $output;
  }

  /**
   * Calculate the size of the video based on given size and type.
   * See Mantis [0016569] for specification.
   *
   * 1.x: _play_proxy_calc_video_size
   *
   * @param integer v_width
   *  mediafile width
   * @param integer v_height
   *  mediafile height
   * @param integer width
   *  play proxy call parameter width
   * @param integer height
   *  play proxy call parameter height
   *
   * @return array
   *  new width en height.
   *
   */
  function calc_video_size($video_width, $video_height, $width, $height, $container_type) {

    // If video size is known, we can do some calculations on height/weight.
    if ($video_width > 0 && $video_height > 0) {
      if ($width > 0 && ($height == 0 || !isset($height))) {
        $height = (int)round($width * ($video_height / $video_width));
      }
      elseif (($width == 0 || !isset($width)) && $height > 0) {
        $width = (int)round($height * ($video_width / $video_height));
      }
      elseif ( ($width == 0 || !isset($width) ) && ($height == 0 || !isset($height)) ) {
        // if none is given we use the default size.
        $width = $video_width;
        $height = $video_height;
      }
    }

    if ($width <= 0 && $height <= 0) {
      // no luck, use default values.
      $width = self::PP_FLASH_OBJECT_WIDTH;
      $height = self::PP_FLASH_OBJECT_HEIGHT;
    }

    // add an extra 64 pixels for windows media buttons.
    switch ($container_type) {
      case self::CONTAINER_TYPE_FLV:
      case self::CONTAINER_TYPE_MP4:
      case self::CONTAINER_TYPE_OGG:
        break;

      case self::CONTAINER_TYPE_WMV:
      default:
        if ($height) {
          $height += 64;
        }
        break;
    }

    return array('width' => $width, 'height' => $height);
  }

  /**
   * Wrapper for STILL response type.
   *
   * @param string $ticket
   * @param integer $app_id
   */
  static public function create_response_still($ticket, $app_id) {
    return self::create_response(self::RESPONSE_STILL, $app_id, $ticket);
  }

  /**
   * Wrapper for 1.x to 2.x.
   * @param array $a_parameters
   * @param array $a_asset_info (used for the metadata)
   * @param array $mediafile_ext
   * @param array $asset_metadata (not used)
   * @param string $ticket
   * @param integer $app_id
   * @param string $mediafile_group_id
   * @param array $stills
   */
  static public function create_response_1x_to_2x($asset_id, $a_parameters, $a_asset_info, $mediafile_ext, $asset_metadata/* unused */, $ticket, $app_id, $stills = array()) {
    return self::create_response(
      $a_parameters['response']['value'],
      $app_id,
      $ticket,
      $asset_id,
      isset($a_parameters['autostart']['value']) ? mediamosa_lib::boolstr2bool($a_parameters['autostart']['value']) : TRUE,
      isset($a_parameters['width']['value']) ? $a_parameters['width']['value'] : 0,
      isset($a_parameters['height']['value']) ? $a_parameters['height']['value'] : 0,
      isset($a_parameters['start']['value']) ? $a_parameters['start']['value'] : NULL,
      isset($a_parameters['duration']['value']) ? $a_parameters['duration']['value'] : NULL,
      isset($a_parameters['is_app_admin']['value']) ? mediamosa_lib::boolstr2bool($a_parameters['is_app_admin']['value']) : FALSE,
      $a_asset_info,
      $mediafile_ext,
      $stills
    );
  }

  /**
   * Generate default still link for asset.
   *
   * @param integer $app_id
   * @param string $asset_id
   */
  static function create_response_still_url($app_id, $asset_id) {

    // Get default still.
    $still_id = mediamosa_asset_mediafile_still::find_default($asset_id);

    // Any still?
    if (!empty($still_id)) {
      $response = mediamosa_asset_mediafile_play_proxy::create_response_still($still_id, $app_id);
      return $response['output'];
    }

    return '';
  }

  /**
   * Create response for mediafile.
   * 1.x params; $a_parameters, $a_asset_info, $mediafile_ext, $asset_metadata, $ticket, $app_id, $mediafile_group_id = '', $stills = array()) {
   *
   * 1.x: _play_proxy_create_response
   *
   * @param string $response_type
   *  See RESPONSE_*.
   * @param integer $app_id
   *  Application ID.
   * @param string $ticket
   *  Precreated ticket.
   * @param integer $autostart
   *  Value for autostart in HTML.
   * @param $width
   *  Value for width in HTML object.
   * @param $height
   *  Value for height in HTML object.
   * @param $start
   *  Start param in HTML object.
   * @param $duration
   *  Duration param in HTML object.
   * @param boolean $is_app_admin
   *  Super admin.
   * @param array $asset_metadata
   *  Collection of the metadata of the asset, use metadata_get function.
   * @param array $mediafile_ext
   *  Mediafile array.
   * @param string $mediafile_group_id
   *  Group ID mediafile.
   * @param $stills
   *  Array stills.
   */
  static public function create_response(
    $response_type,
    $app_id,
    $ticket,
    $asset_id = NULL,
    $autostart = TRUE,
    $width = 0,
    $height = 0,
    $start = NULL,
    $duration = NULL,
    $is_app_admin = FALSE,
    array $asset_metadata = array(),
    array $mediafile_ext = array(),
    array $stills = array()
  ) {

    // Not an array here.
    assert(!is_array($app_id));

    $content_type = '';

    if ($response_type == self::RESPONSE_METAFILE) {
      $tickets = $ticket;
      $mediafile_exts = $mediafile_ext;
      $mediafile_ext = reset($mediafile_exts);
      $streaming_server_uri = self::get_media_server($response_type, $mediafile_ext);
    } else {
      if (isset($mediafile_ext[mediamosa_asset_mediafile_db::URI]) && trim($mediafile_ext[mediamosa_asset_mediafile_db::URI]) != '') {
        $streaming_server_uri = trim($mediafile_ext[mediamosa_asset_mediafile_db::URI]);
      }
      else {
        $streaming_server_uri = self::get_media_server($response_type, $mediafile_ext);
      }

      $streaming_server_ticket_uri = strtr($streaming_server_uri, array(self::PARAM_URI_TICKET => $ticket));

      // Replace {TICKET} with app_id / ticket.
      if ($response_type != self::RESPONSE_STILL) {
        $streaming_server_ticket_uri = strtr($streaming_server_uri, array(self::PARAM_URI_TICKET => $app_id . '/' . $ticket));
      }

      // Uri for non-tickets (still, download etc).
      foreach ($stills as $key => $value) {
        $stills[$key]['still_ticket'] = strtr($streaming_server_uri, array(self::PARAM_URI_TICKET => $stills[$key]['ticket']));
      }
    }

    // So it exists.
    $variables = array();
    $output = '';

    switch ($response_type) {
      case self::RESPONSE_METAFILE:

        switch ($mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_MP4:
            if ($mediafile_ext['metadata']['video_codec'] == self::VIDEO_CODEC_H264) {

              // Get the rtmp (wowza) server URI.
              // URI should looks like:
              // rtmp://wowstreaming.test.snkn.nl/vod&file=vpx-acc/{TICKET}
              // rtmp://wowstreaming.snkn.nl/simplevideostreaming&file=vpx/{TICKET}
              // Separate the URL into two parts:
              // "rtmp://wowstreaming.test.snkn.nl/vod" and "vpx-acc/{TICKET}"

              $exploded_uri = explode('&file=', $streaming_server_uri);
              if (isset($exploded_uri[1])) {
                $uri_base = $exploded_uri[0];
                $uri_src = $exploded_uri[1];
              }

              else {
                // Apache server.
                // http://mediamosa2/ticket/LRQKJp4RRHdiOQjZJAiV2rOP.flv
                // Separation by the last '/':
                // "http://mediamosa2/ticket/" and "LRQKJp4RRHdiOQjZJAiV2rOP.flv"
                $last_index = strrpos($streaming_server_ticket_uri, '/');
                $uri_base = mediamosa_unicode::substr($streaming_server_ticket_uri, 0, $last_index + 1);
                $uri_src = mediamosa_unicode::substr($streaming_server_ticket_uri, $last_index + 1);
              }
              // Create the output.
              $output = '<smil>' . "\n";
              $output .= '  <head>' . "\n";
              $output .= '    <meta base="' . $uri_base . '" />' . "\n";
              $output .= '  </head>' . "\n";
              $output .= '    <body>' . "\n";
              $output .= '      <switch/>' . "\n";
              $i = 0;
              foreach($tickets as $ticket)  {
                $streaming_server_ticket_uri = strtr($streaming_server_uri, array(self::PARAM_URI_TICKET => $app_id . '/' . $ticket));
                $exploded_uri = explode('&file=', $streaming_server_ticket_uri);
                  if (isset($exploded_uri[1])) {
                  $uri_base = $exploded_uri[0];
                  $uri_src = $exploded_uri[1];
                }
                $output .= '        <video src="' . $uri_src . '" '. 'system-bitrate="' . $mediafile_exts[$i]['metadata']['bitrate'] . '" ' . 'width="' . $mediafile_exts[$i]['metadata']['width'] . '"/>' . "\n";
                $i++;
              }
              $output .= '      </switch>' . "\n";
              $output .= '   </body>' . "\n";
              $output .= '</smil>';
              // Content type.
              $content_type = 'application/smil';
            }
            else {
              $output = '<?xml version="1.0"?>' . "\n";
              $output .= '<?quicktime type="application/x-quicktime-media-link"?>' . "\n";
              $output .= sprintf("<embed autoplay=\"%s\" src=\"%s\" />\n", $autostart ? 'true' : 'false', $streaming_server_ticket_uri);
              $content_type = 'application/x-quicktimeplayer';
            }
            break;

          case self::CONTAINER_TYPE_MP3:
            $content_type = 'audio/mpeg';
            $output = "<asx version=\"3.0\">\n\t<entry>\n";

            if (!empty($asset_metadata['dublin_core'])) {
              foreach (array('TITLE' => 'title', 'COPYRIGHT' => 'rights', 'ABSTRACT' => 'description', 'AUTHOR' => 'creator') as $tag => $subject) {
                if (!empty($asset_metadata['dublin_core'][$subject])) {
                  $output .= "\t\t<" . $tag . '>' . implode(' ', $asset_metadata['dublin_core'][$subject]['values']) . '</' . $tag . ">\n";
                }
              }
            }

            $output .= sprintf("\t\t<ref href=\"%s\" />\n", $streaming_server_ticket_uri);
            $output .= "\t</entry>\n</asx>\n";
            break;

          case self::CONTAINER_TYPE_WMV:
            $content_type = 'video/x-ms-asf';
            $output = "<asx version=\"3.0\">\n\t<entry>\n";

            if (!empty($asset_metadata['dublin_core'])) {
              foreach (array('TITLE' => 'title', 'COPYRIGHT' => 'rights', 'ABSTRACT' => 'description', 'AUTHOR' => 'creator') as $tag => $subject) {
                if (!empty($asset_metadata['dublin_core'][$subject])) {
                  $output .= "\t\t<" . $tag . '>' . implode(' ', $asset_metadata['dublin_core'][$subject]['values']) . '</' . $tag . ">\n";
                }
              }
            }

            // Add start and duration to asx.
            if (isset($start)) {
              $output .= sprintf("\t\t<starttime value=\"%s\" />\n", self::convert_msec($start, self::CONTAINER_TYPE_WMV));
            }

            if (isset($duration)) {
              $output .= sprintf("\t\t<duration value=\"%s\" />\n", self::convert_msec($duration, self::CONTAINER_TYPE_WMV));
            }

            $output .= sprintf("\t\t<ref href=\"%s\" />\n", $streaming_server_ticket_uri);
            $output .= "\t</entry>\n</asx>\n";
            break;

          case self::CONTAINER_TYPE_FLV:

            // Get the rtmp (wowza) server URI.
            // URI should looks like:
            // rtmp://wowstreaming.test.snkn.nl/vod&file=vpx-acc/{TICKET}
            // rtmp://wowstreaming.snkn.nl/simplevideostreaming&file=vpx/{TICKET}
            // Separate the URL into two parts:
            // "rtmp://wowstreaming.test.snkn.nl/vod" and "vpx-acc/{TICKET}"
            $exploded_uri = explode('&file=', $streaming_server_ticket_uri);
            if (isset($exploded_uri[1])) {
              $uri_base = $exploded_uri[0];
              $uri_src = $exploded_uri[1];
            }
            else {
              // Apache server.
              // http://mediamosa2/ticket/LRQKJp4RRHdiOQjZJAiV2rOP.flv
              // Separation by the last '/':
              // "http://mediamosa2/ticket/" and "LRQKJp4RRHdiOQjZJAiV2rOP.flv"
              $last_index = strrpos($streaming_server_ticket_uri, '/');
              $uri_base = mediamosa_unicode::substr($streaming_server_ticket_uri, 0, $last_index + 1);
              $uri_src = mediamosa_unicode::substr($streaming_server_ticket_uri, $last_index + 1);
            }
            // Create the output.
            $output = '<smil>' . "\n";
            $output .= '  <head>' . "\n";
            $output .= '    <meta base="' . $uri_base . '" />' . "\n";
            $output .= '  </head>' . "\n";
            $output .= '  <body>' . "\n";
            $output .= '    <video src="' . $uri_src . '" />' . "\n";
            $output .= '  </body>' . "\n";
            $output .= '</smil>';
            // Content type.
            $content_type = 'application/smil';
            break;

          default:
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_METAFILE_AVAILABLE, array('@container_type' => $mediafile_ext['metadata']['container_type']));
        }
        break;

      case self::RESPONSE_URI:
        switch ($mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_FLV:
            $output = str_replace(self::PARAM_URI_SCRIPT, 'StroboScope.php?file=', $streaming_server_ticket_uri);
            break;

          default:
            $output = $streaming_server_ticket_uri;
            break;
        }
        break;

      case self::RESPONSE_DOWNLOAD:
        if (!$is_app_admin) {
          if (isset($mediafile_ext['is_downloadable']) && !mediamosa_lib::boolstr2bool($mediafile_ext['is_downloadable'])) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_DOWNLOAD_DISABLED);
          }
        }

        if (!@is_null($mediafile_ext[mediamosa_asset_mediafile_db::URI])) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STREAM_DOWNLOAD_NOT_ALLOWED);
        }

        if ($mediafile_ext['filename'] == '') {
          $mediafile_ext['filename'] = mediamosa_asset_mediafile::UNKNOWN_FILENAME;
        }

        $output = $streaming_server_uri . '/' . rawurlencode($mediafile_ext['filename']);
        break;

      case self::RESPONSE_OBJECT:
        $video_size = self::calc_video_size(
          isset($mediafile_ext['metadata']['width']) ? $mediafile_ext['metadata']['width'] : 0,
          isset($mediafile_ext['metadata']['height']) ? $mediafile_ext['metadata']['height'] : 0,
          $width,
          $height,
          isset($mediafile_ext['metadata']['container_type']) ? $mediafile_ext['metadata']['container_type'] : self::CONTAINER_TYPE_WMV
        );

        // Get the Object code.
        $output = mediamosa_server::get_objectcode_streaming($mediafile_ext['metadata']['container_type'], $mediafile_ext['metadata']['video_codec'], $mediafile_ext['metadata']['mime_type']);

        $variables = array(
          'WIDTH' => !$video_size['width'] ? '' : $video_size['width'],
          'HEIGHT' => !$video_size['height'] ? '' : $video_size['height'],
          'HEIGHT_PLUS_20' => $video_size['height'] > 0 ? $video_size['height'] + 20 : '',
          'MEDIAFILE_ID' => $mediafile_ext['mediafile_id'],
          'TICKET_URI' => $streaming_server_ticket_uri,
          'STILL_URI' => self::create_response_still_url($app_id, $asset_id),
          'AUTOPLAY' => $autostart ? 'true' : 'false',
          'AUTOPLAY_NUM' => $autostart ? '1' : '0',
          'AUTOPLAY_TEXT' => $autostart ? 'autoplay' : '',
          'IF_START' => '', '/IF_START' => '',
          'IF_DURATION' => '', '/IF_DURATION' => '',
          'IF_EXTERNAL' => '', '/IF_EXTERNAL' => '',
        );

        // HEIGHT_PLUS_num.
        $pattern = '/{HEIGHT_PLUS_(\d+)}/';
        $output = preg_replace_callback(
          $pattern,
          create_function(
            '$matches',
            'return ' . ($video_size['height'] > 0 ? $video_size['height'] . ' + $matches[1]' : '""') . ';'
          ),
          $output
        );

        // START, END DURATION TIMES variables
        if (isset($start)) {
          $variables['START_TIME'] = self::convert_msec($start, $mediafile_ext['metadata']['container_type']);
          $variables['START_TIME_SECONDS'] = (int)($start / 1000);
        }
        else {
          // Remove code {IF_START} (Exclude from output) //   \{IF_START\}(.*?)\{/IF_START\}/is
          $output = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output);
        }

        if (isset($duration)) {
          $endtime = $start + $duration;
          $variables['DURATION_TIME'] = self::convert_msec($duration, $mediafile_ext['metadata']['container_type']);
          $variables['DURATION_TIME_SECONDS'] = (int)($duration/1000);
          $variables['START_PLUS_DURATION_TIME_SECONDS'] = (int)($start + $duration)/1000;
          $variables['END_TIME'] = self::convert_msec($endtime, $mediafile_ext['metadata']['container_type']);
        }
        else {
          $output = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output);
        }

        $content_type = 'text/html';

        switch ($mediafile_ext['metadata']['container_type']) {
          case self::CONTAINER_TYPE_FLV:
          case self::CONTAINER_TYPE_MP4:
          case self::CONTAINER_TYPE_MP3:
          case self::CONTAINER_TYPE_OGG:
            break;

          case self::CONTAINER_TYPE_WMV:
          default:
            // Either set, then we must use an external asx file.
            $use_external_asx_file = (isset($start) || isset($duration));

            // If TRUE, create an asx file and when everything goes as planned, ref to the new file.
            if ($use_external_asx_file) {
              preg_match('@\{IF_EXTERNAL\}(.*?)\{/IF_EXTERNAL\}@is', $output, $input);
              if (isset($input[0])) {
                $output_asx = $input[0];
              }

              if (isset($start)) {
                // Remove code {IF_START} (Exclude from output)
                $variables['START_TIME'] = self::convert_msec($start, self::CONTAINER_TYPE_WMV);
              }
              else {
                $output_asx = preg_replace('@\{IF_START\}.*?\{/IF_START\}@si', '', $output_asx);
              }

              if (isset($duration)) {
                $variables['DURATION_TIME'] = self::convert_msec($duration, self::CONTAINER_TYPE_WMV);
              }
              else {
                $output_asx = preg_replace('@\{IF_DURATION\}.*?\{/IF_DURATION\}@si', '', $output_asx);
              }

              // Process all the variables and make replacement in the objectcode.
              foreach ($variables as $key => $value) {
                $output_asx = str_replace('{' . $key . '}', $value, $output_asx);
              }

              // Get the filename for the asx file.
              $file_asx = mediamosa_configuration_storage::asx_file_location($ticket);

              // Open file for writing.
              $handle_file_asx = @fopen($file_asx, 'w+');

              // Check if file handle is open, else log it.
              if ($handle_file_asx) {
                // Write to file.
                $bytes_written = @fwrite($handle_file_asx, $output_asx);

                // Close file handle.
                fclose($handle_file_asx);

                // Any bytes written.
                if ($bytes_written) {
                  $uri = mediamosa_server::get_uri_still();

                  // If not found, then throw an error.
                  if ($uri == FALSE) {
                    throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_STILL_NOT_FOUND);
                  }

                  // Create new streaming server URI based on asx file.
                  $streaming_server_ticket_uri = str_replace(self::PARAM_URI_TICKET, urlencode($ticket) . '.asx', $uri);

                  // Changed to text/html for ticket #188.
                  //'video/x-ms-asx';
                  $content_type = 'text/html';
                }
                else {
                  mediamosa_watchdog::log('Unable to write to .asx file (@filename), falling back to normal object code', array('@filename' => $file_asx), WATCHDOG_CRITICAL, 'playproxy');
                }
              }
              else {
                mediamosa_watchdog::log('Unable to create .asx file (@filename), falling back to normal object code', array('@filename' => $file_asx), WATCHDOG_CRITICAL, 'playproxy');
              }
            }

            $variables['WMA_TICKET_URI'] = $streaming_server_ticket_uri;

            // Remove code {IF_EXTERNAL} (Exclude from output)
            $output = preg_replace('@\{IF_EXTERNAL\}.*?\{/IF_EXTERNAL\}@si', '', $output);
            break;
        }
        break;

      case self::RESPONSE_STILL:
        $output = $streaming_server_uri;
        break;
    }

    // Process all the variables and make replacement in the objectcode
    foreach ($variables as $key => $value) {
      $output = str_replace('{' . $key . '}', $value, $output);
    }

    $result = array(
      'output' => $output,
      'content_type' => $content_type,
      'ticket_id' => $ticket,
    );

    $i = 1;
    foreach ($stills as $key => $value) {
      $result['stills']['#' . serialize(array('id' => $i++))] = $value;
    }

    return $result;
  }

  /**
   * Return a matching mediafile based on the asset_id and a profile_id.
   *
   * 1.x: _play_proxy_get_mediafile_id_on_profile
   *
   * @param $asset_id
   * @param $profile_id
   * @param $original_mediafile_id
   */
  static public function get_mediafile_id_on_profile($asset_id, $profile_id, $original_mediafile_id = NULL) {

    // No original mediafile?
    if (!isset($original_mediafile_id)) {
      return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
        ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
        ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
        ->condition(mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, $profile_id)
        ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE)
        ->range(0, 1)
        ->execute()
        ->fetchField();
    }

    // Get the specific transcoded mediafile with provided original mediafile
    // and its profile id.
    return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition(mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, $profile_id)
      ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE)
      ->condition(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $original_mediafile_id)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }

  /**
   * Return a matching mediafile based on the asset_id and a tag.
   *
   * @param $asset_id
   * @param $tag
   */
  static public function get_mediafile_id_on_tag($asset_id, $tag) {
    return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE)
      ->condition(mediamosa_asset_mediafile_db::TAG, $tag)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }

  /**
   * Check if the time has been restricted.
   *
   * @param string $time_restriction_start
   * @param string $time_restriction_end
   */
  function check_time_restrictions($time_restriction_start, $time_restriction_end) {
    if (time() < $time_restriction_start) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TIME_RESTRICTION_START, array('@date' => date("Y-m-d H:i:s", $time_restriction_start), '@timestamp' => $time_restriction_start));
    }
    if (time() > $time_restriction_end) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_TIME_RESTRICTION_END, array('@date' => date("Y-m-d H:i:s", $time_restriction_end), '@timestamp' => $time_restriction_end));
    }
  }
}
