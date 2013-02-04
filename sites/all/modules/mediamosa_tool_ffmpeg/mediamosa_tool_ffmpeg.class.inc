<?php
/**
 * @file
 * FFmpeg tool functions.
 */

class mediamosa_tool_ffmpeg {
  // ------------------------------------------------------------------- Consts.
  /**
   * Name / ID of the module.
   */
  const NAME = 'ffmpeg';

  /**
   * Video duration max value in second.
   */
  const VIDEO_DURATION_MAX = 99999;

  /**
   * Black still interval.
   */
  const BLACK_STILL_INTERVAL = 30;

  // ---------------------------------------------------------------- Functions.
  /**
   * Test if ffmpeg uses -padtop or -vf=pad function in this version.
   *
   * @param $set_value
   *   Force the value, used during unit testing.
   *
   * @return
   *   Returns TRUE when ffmpeg option -padtop can not be used by ffmpeg and you
   *   must use the -vf (video filter) option.
   */
  public static function useVideoFilter($set_value = NULL) {
    static $useVideoFilter = NULL;

    // Force the value?
    if (isset($set_value)) {
      $useVideoFilter = $set_value;
    }

    if (!isset($useVideoFilter)) {
      $ffmpeg_response = exec('ffmpeg -padtop 0 2>&1');
      $useVideoFilter = strpos($ffmpeg_response, 'vf=pad') !== FALSE;
      if (!$useVideoFilter) {
        $useVideoFilter = (strpos(drupal_strtolower($ffmpeg_response), 'failed') !== FALSE);
      }
    }

    return $useVideoFilter;
  }

  /**
   * Calculate the aspect ratio.
   *
   * Can return cmd line option in ffmpeg when needed.
   *
   * @param integer $source_width
   *   Width of source.
   * @param integer $source_height
   *   Height of source.
   * @param string $target_size
   *   The size of the target in WIDTHxHEIGHT format.
   * @param integer $h_padding
   *   Horizontal padding.
   * @param integer $v_padding
   *   Vertical padding.
   * @param boolean $use_padding
   *   Use padding parameters.
   *
   * @return array|FALSE
   *   Returns commands array or FALSE when failure.
   */
  public static function calcAspectRatio($source_width, $source_height, $target_size, $h_padding = NULL, $v_padding = NULL, $use_padding = TRUE) {

    // Calculate ratio.
    $aspect_ratio = mediamosa_gd::calcAspectRatio($source_width, $source_height, $target_size, $h_padding, $v_padding, $use_padding);
    if (!$aspect_ratio) {
      return FALSE;
    }

    // Does current ffmpeg use new videofilter (-vf)?
    if (!mediamosa_tool_ffmpeg::useVideoFilter()) {
      return array(
        '-s' => $aspect_ratio['width'] . 'x' . $aspect_ratio['height'],
        '-padtop' => (int) $aspect_ratio['h_padding'],
        '-padbottom' => (int) $aspect_ratio['h_padding'],
        '-padleft' => $aspect_ratio['v_padding'],
        '-padright' => $aspect_ratio['v_padding'],
      );
    }

    return array(
      '-s' => $aspect_ratio['width'] . 'x' . $aspect_ratio['height'],
      '-vf' => strtr(
        'pad=@width:@height:@v_padding:@h_padding:black',
        array(
          '@width' => $aspect_ratio['width'] + ($aspect_ratio['v_padding'] * 2),
          '@height' => $aspect_ratio['height'] + ($aspect_ratio['h_padding'] * 2),
          '@v_padding' => $aspect_ratio['v_padding'],
          '@h_padding' => $aspect_ratio['h_padding'],
        )
      )
    );
  }

  /**
   * Get the exec string for generating a still.
   *
   * @deprecated: Although new, its deprecated. We will replace this function
   *              with generate_still() very soon and no longer rely on exec
   *              strings return. Rewrite of jobs is planned for v3.5.
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id_source
   */
  public static function get_generate_still_exec($jobserver_job_id, $mediafile_id_source) {

    $job_server_still = mediamosa_job_server_still::get($jobserver_job_id);
    $job_server = mediamosa_job_server::get($jobserver_job_id);
    $job_id = $job_server[mediamosa_job_server_db::JOB_ID];

    $execution_string = '';

    if ($job_server_still) {
      $frametime = (int) $job_server_still[mediamosa_job_server_still_db::FRAMETIME];
      $size = $job_server_still[mediamosa_job_server_still_db::SIZE];
      $h_padding = $job_server_still[mediamosa_job_server_still_db::H_PADDING];
      $v_padding = $job_server_still[mediamosa_job_server_still_db::V_PADDING];

      $still_parameters = unserialize($job_server_still[mediamosa_job_server_still_db::STILL_PARAMETERS]);
      $fps = (int) $still_parameters['fps'];
      $video_duration = (int) $still_parameters['video_duration'];

      // Video duration check, if still_type = scene.
      if (!$video_duration && $still_parameters['still_type'] == mediamosa_asset_mediafile_metadata::STILL_TYPE_SCENE) {
        // Some videos don't have video durations according to FFMpeg.
        // If we want to create multiple stills, we usually need this information calculating other data.
        // But in case of scene detection we have a work around, we can put the video duration on a maximum value.
        mediamosa_job_server::log_debug_high_mediafile($mediafile_id_source, t('No video duration, when still_type = scene. Try to get the complete video. video_duration = @video_duration.', array(
          '@video_duration' => print_r($video_duration, TRUE),
        )));
        $video_duration = self::VIDEO_DURATION_MAX;
      }

      $start_frame = min(max(0, (int) $still_parameters['start_frame']), max(0, $video_duration - 1));
      if (isset($still_parameters['end_frame']) && (int) $still_parameters['end_frame']) {
        $end_frame = min(max(0, (int) $still_parameters['end_frame']), max(0, $video_duration - 1));
      }
      else {
        $end_frame = max(0, $video_duration - 1);
      }

      // Make sure start and end are in the correct order from low to high.
      $start_frame_cpy = $start_frame;
      $start_frame = min($start_frame, $end_frame);
      $end_frame = max($start_frame_cpy, $end_frame);

      // When video_duration is 0, then fall back to the original behaviour.
      if (!$video_duration || $start_frame == $end_frame) {
        $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
        mediamosa_job_server::log_debug_high_mediafile($mediafile_id_source, t('No video duration or start frame = end frame. Fall back to still_type = NONE. video_duration = @video_duration, start_frame = @start_frame, end_frame = @end_frame.', array(
          '@video_duration' => print_r($video_duration, TRUE),
          '@start_frame' => print_r($start_frame, TRUE),
          '@end_frame' => print_r($end_frame, TRUE),
        )));
      }

      // Make sure still_type is uppercase.
      $still_parameters['still_type'] = mediamosa_unicode::strtoupper($still_parameters['still_type']);

      // Data check
      switch ($still_parameters['still_type']) {
        case 'NORMAL':
          if (!is_numeric($still_parameters['still_per_mediafile']) || $still_parameters['still_per_mediafile'] < 1) {
            $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
            $error_description = 'Still_per_mediafile parameter is missing. Still creation fall back to still_type = NONE';
            mediamosa_job_server::log_debug_high_mediafile($mediafile_id_source, t('Still_per_mediafile parameter is missing. Still creation fall back to still_type = NONE.'));
          }
          break;

        case 'SECOND':
          if (!is_numeric($still_parameters['still_every_second']) || $still_parameters['still_every_second'] < 0) {
            $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
            $error_description = 'Still_every_second parameter is missing. Still creation fall back to still_type = NONE';
            mediamosa_job_server::log_debug_high_mediafile($mediafile_id_source, t('Still_every_second parameter is missing. Still creation fall back to still_type = NONE.'));
          }
          break;

        case 'SCENE':
          if (!$fps) {
            $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
            $error_description = 'FPS parameter is 0. Still creation fall back to still_type = NONE';
            mediamosa_job_server::log_debug_high_mediafile($mediafile_id_source, t('FPS parameter is 0. Still creation fall back to still_type = NONE.'));
          }
          break;

        default:
          break;
      }

      // Store error_description.
      if (!empty($error_description)) {
        mediamosa_job_server::log_mediafile($mediafile_id_source, $error_description);
        mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
          ->fields(array(
            mediamosa_job_server_db::ERROR_DESCRIPTION => $error_description,
          ))
          ->condition(mediamosa_job_server_db::ID, $jobserver_job_id)
          ->execute();
      }

      // Frametime but be between start and end frame. Put it roughly 1 / 3 in
      // length.
      if ($frametime < $start_frame || $frametime > $end_frame) {
        $frametime = (($end_frame - $start_frame) / 3) + $start_frame;
      }

      $duration = $framerate = 1;

      switch (mediamosa_unicode::strtoupper($still_parameters['still_type'])) {

        case 'NONE':
          if ($job_server_still[mediamosa_job_server_still_db::BLACKSTILL_CHECK] == mediamosa_job_server_still_db::BLACKSTILL_CHECK_FALSE || $video_duration <= 1) {
            break;
          }

          $still_parameters['still_per_mediafile'] = 5;
          $start_frame = max($frametime - self::BLACK_STILL_INTERVAL, 0);
          $end_frame = min($frametime + self::BLACK_STILL_INTERVAL, $video_duration - 1);
          // Go to the NORMAL now.

        case 'NORMAL':
          $still_per_mediafile = max($still_parameters['still_per_mediafile'], 1);
          $frametime = $start_frame;
          $duration = ($end_frame - $start_frame);
          // |--------- video length -----------|
          // Stills per mediafile = 4
          // |------S------S------S------S------|
          // $framesecond = duration / (4 + 1)
          // .......|-- video length -----------|
          // frametime += $framesecond
          // duration -= $framesecond
          $framesecond = $duration / ($still_per_mediafile + 1);
          $frametime += $framesecond;
          $duration -= $framesecond;
          // Frames per second
          $framerate = 1 / max($framesecond, 1);
          // Safety check
          if ($duration / max($framesecond, 1) > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $framesecond;
          }
          break;

        case 'SECOND':
          $still_every_second = $still_parameters['still_every_second'];
          $frametime = $start_frame;
          $duration = ($end_frame - $start_frame);
          $framerate = 1 / $still_every_second;
          // Safety check
          if ($duration / $still_every_second > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $still_every_second;
          }
          break;

        case 'SCENE':
          mediamosa_job_server::log_mediafile($mediafile_id_source, 'Scene detection starts');
          $destination_path = mediamosa_storage::get_realpath_temporary();
          $input = mediamosa_storage::get_realpath_mediafile($mediafile_id_source);

          // Clean up I.
          mediamosa_io::unlink($destination_path . $mediafile_id_source . '_scene.avi');
          mediamosa_io::unlink($destination_path . $mediafile_id_source . '_list.el');


  /*
  // Old code
          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $input, $destination_path . $mediafile_id_source .'_scene.avi');
          //mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
          exec($order);
  */

  // Hot fix
          $order = sprintf('ffmpeg -i %s %s', $input, $destination_path . $mediafile_id_source . '_scene.wmv');
          //mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
          mediamosa_io::exec($order);

          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $destination_path . $mediafile_id_source . '_scene.wmv', $destination_path . $mediafile_id_source . '_scene.avi');
          //mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
          mediamosa_io::exec($order);

          mediamosa_io::unlink($destination_path . $mediafile_id_source . '_scene.wmv');
  // Hot fix - end

          $order = sprintf('lav2yuv -T 1 -S %s %s', $destination_path . $mediafile_id_source . '_list.el', $destination_path . $mediafile_id_source . '_scene.avi');
          mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
          mediamosa_io::exec($order);

          $order = sprintf('cat %s', $destination_path . $mediafile_id_source . '_list.el');
          //mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
          $output = mediamosa_io::exec($order);
          //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('output = %s', print_r($output, TRUE)));

          // Clean up II.
          mediamosa_io::unlink($destination_path . $mediafile_id_source . '_list.el');

          // Analyze
          if (!empty($output)) {
            //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('Output is array with data'));

            // tbr & map from the best stream
            $map = NULL;
            $tbr = NULL;
            $kbs = NULL;
            $order = sprintf('ffmpeg -i %s 2>&1', $input);
            //mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
            $details = mediamosa_io::exec($order);

            $regs = $reg_kbs = $reg_map = array();
            foreach ($details as $line) {
              if (stripos($line, ' Stream ') !== FALSE && stripos($line, ' Video: ') !== FALSE) {
                if (preg_match('/Stream #[0-9]*\.([0-9]*)/', $line, $reg_map) && preg_match('/ ([0-9]*) kb\/s/', $line, $reg_kbs)) {
                  if (!$map || !$kbs || $kbs < $reg_kbs[1]) {
                    $map = $reg_map[1];
                    $kbs = $reg_kbs[1];
                  }
                  if (preg_match('/ ([0-9]*(\.[0-9]*)+) tb/', $line, $regs)) {
                    $tbr = $regs[1];
                  }
                }
                elseif (!$map && preg_match('/ ([0-9]*(\.[0-9]*)+) tb/', $line, $regs)) {
                  $tbr = $regs[1];
                  if (preg_match('/Stream #[0-9]*\.([0-9]*)/', $line, $reg_map)) {
                    $map = $reg_map[1];
                  }
                }
              }
            }
            //mediamosa_job_server::log_mediafile($mediafile_id_source, 'tbr: '. print_r($tbr, TRUE) .', map: '. print_r($map, TRUE) .', kbs: '. print_r($kbs, TRUE));
            if (!$tbr) {
              // This is the base value
              $tbr = 25;
            }
            // Go further, if we have tbr
            if ($tbr) {

              $scene_frame = NULL;
              $i = 1;
              $scenes = array();
              foreach ($output as $line) {
                //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('Line: %s', print_r($line, TRUE)));
                $line_args = explode(' ', $line);
                //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('Line args: %s', print_r($line_args, TRUE)));
                if ($line_args && is_array($line_args) && $line_args != array()) {
                  //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('Line args is array with data'));
                  if (isset($line_args[0]) && isset($line_args[1]) && isset($line_args[2]) && !isset($line_args[3]) && is_numeric($line_args[0]) && is_numeric($line_args[1]) && is_numeric($line_args[2])) {
                    //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('Line args has 3 numbers'));
                    // $line_args[1] + num = a little bit after the change
                    $change_frame = $line_args[1] + mediamosa_settings::STILL_SCENE_AFTER;
                    //mediamosa_job_server::log_mediafile($mediafile_id_source, 'change_frame: '. print_r($change_frame, TRUE) .', start_frame: '. print_r($start_frame, TRUE) .', fps = '. $fps .', end_frame: '. print_r($end_frame, TRUE));
                    if ($change_frame >= ($start_frame*$fps) && $change_frame <= ($end_frame*$fps)) {
                      // $scene_frame + num = minimal distance with two changes
                      //mediamosa_job_server::log_mediafile($mediafile_id_source, 'scene_frame: '. print_r($scene_frame, TRUE));
                      if (!$scene_frame || $scene_frame + mediamosa_settings::STILL_SCENE_DISTANCE < $change_frame) {
                        $scene_frame = $change_frame;
                        //mediamosa_job_server::log_mediafile($mediafile_id_source, sprintf('New scene frame: %s', $scene_frame));
                        // The avi has 25 frame/sec
                        $scene_sec = (int)($scene_frame/25);
                        $scenes[] = $scene_sec;
                        $i++;
                        if ($i > mediamosa_settings::STILL_MAXIMUM) {
                          // Emergency break
                          break;
                        }
                      }
                    }
                  }
                }
              }
            }
          }

          // Save the sec data (see: vpx_jobhandler.module)
          if (is_array($scenes) && $scenes != array()) {
            $my_file = $destination_path . $job_id . '_ffmpeg_scene.txt';
            $fh = fopen($my_file, 'w') or $fh = NULL;
            if ($fh) {
              foreach ($scenes as $scene) {
                fwrite($fh, $scene . "\n");

              }
              fclose($fh);

              // This version is creating stills for the original video
              if (!mediamosa_tool_ffmpeg::useVideoFilter()) {
                $order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $input, $size, $h_padding, $h_padding, $v_padding, $v_padding, $my_file, ($map ? '-map 0.' . $map : ''), $destination_path . $mediafile_id_source . mediamosa_settings::STILL_EXTENSION . '.jpeg');
              }
              else {
                $size_explode = explode('x', $size);
                $size_width = ($v_padding > 0) ? $size_explode[0] + (2 * $v_padding) : 0;
                $size_height = ($h_padding > 0) ? $size_explode[1] + (2 * $h_padding) : 0;
                $order = sprintf(mediamosa_settings::STILL_SCENE_STRING_VFPAD, $input, $size, $size_width, $size_height, $v_padding, $h_padding, $my_file, ($map ? '-map 0.' . $map : ''), $destination_path . $mediafile_id_source . mediamosa_settings::STILL_EXTENSION . '.jpeg');
              }

              // This version is creating stills for the analysed file
              //$order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $destination_path . $mediafile_id_source .'_scene.avi', $size, $h_padding, $h_padding, $v_padding, $v_padding, $my_file, '', $destination_path . $mediafile_id_source . mediamosa_settings::STILL_EXTENSION .".jpeg");
              mediamosa_job_server::log_mediafile($mediafile_id_source, $order);
              $details = mediamosa_io::exec($order);
              //mediamosa_job_server::log_mediafile($mediafile_id_source, 'Details: '. print_r($details, TRUE));

              mediamosa_io::unlink($my_file);
            }

            // Clean up III.
            mediamosa_io::unlink($destination_path . $mediafile_id_source . '_scene.avi');

            // It has to be the last, because it triggers the further processes
            $my_file = $destination_path . $job_id . '_scene.txt';
            $fh = fopen($my_file, 'w') or $fh = NULL;
            if ($fh) {
              foreach ($scenes as $scene) {
                fwrite($fh, $scene . "\n");
              }
              fclose($fh);
            }

            // Do nothing after this
            return 'echo "Scene stills are rock"';
          }

          // Clean up IV.
          mediamosa_io::unlink($destination_path . $mediafile_id_source . '_scene.avi');

          // We reached this point, so something went wrong in the creation of scene stills.
          // So we are creating still type NONE (original behaviour).
          break;

        default:
          break;
      }

    }
    else {
      mediamosa_job_server::log_mediafile($mediafile_id_source, 'Still job not found, jobserver_job_id: @jobserver_job_id', array('@jobserver_job_id' => $jobserver_job_id));
      return $execution_string;
    }

    $still_parameters['frametime'] = $frametime;
    $still_parameters['duration'] = $duration;
    $still_parameters['framerate'] = $framerate;

    $fields = array(
      mediamosa_job_db::STILL_PARAMETERS => serialize($still_parameters),
    );

    // Enrich with changed/created.
    $fields = mediamosa_db::db_update_enrich($fields);
    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute();

    // Combine the string (only for linux, windows does not generate stills).
    if (!mediamosa_tool_ffmpeg::useVideoFilter()) {
      $execution_string = sprintf('%s %s %s %s %s jpeg ' . mediamosa_settings::STILL_STRING, mediamosa_settings::lua_transcode_script(), mediamosa_storage::get_local_mediafile_path($mediafile_id_source), mediamosa_storage::get_realpath_temporary(), $mediafile_id_source, $job_id, $size, $h_padding, $h_padding, $v_padding, $v_padding,
      round($duration * $framerate),
      $frametime, $duration, $framerate);
    }
    else {
      $size_explode = explode('x', $size);
      $size_width = ($v_padding > 0) ? $size_explode[0] + (2 * $v_padding) : 0;
      $size_height = ($h_padding > 0) ? $size_explode[1] + (2 * $h_padding) : 0;
      $execution_string = sprintf('%s %s %s %s %s jpeg ' . mediamosa_settings::STILL_STRING_VFPAD, mediamosa_settings::lua_transcode_script(), mediamosa_storage::get_local_mediafile_path($mediafile_id_source), mediamosa_storage::get_realpath_temporary(), $mediafile_id_source, $job_id, $size, $size_width, $size_height, $v_padding, $h_padding,
      round($duration * $framerate), $frametime, $duration, 1);
    }
    $execution_string .= ' > /dev/null &';
    return $execution_string;
  }
}
