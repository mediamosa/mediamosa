/**
 * Implementation of Drupal behavior.
 */
(function($) {
Drupal.behaviors.rubik = {};
Drupal.behaviors.rubik.attach = function(context, settings) {
  // If there are both main column and side column buttons, only show the main
  // column buttons if the user scrolls past the ones to the side.
  $('div.form:has(div.column-main div.form-actions):not(.rubik-processed)', context).each(function() {
    var form = $(this);
    var offset = $('div.column-side div.form-actions', form).height() + $('div.column-side div.form-actions', form).offset().top;
    $(window).scroll(function() {
      if ($(this).scrollTop() > offset) {
        $('div.column-main .column-wrapper > div.form-actions#edit-actions', form).show();
      }
      else {
        $('div.column-main .column-wrapper > div.form-actions#edit-actions', form).hide();
      }
    });
    form.addClass('rubik-processed');
  });

  $('a.toggler:not(.rubik-processed)', context).each(function() {
    var id = $(this).attr('href').split('#')[1];
    // Target exists, add click handler.
    if ($('#' + id).size() > 0) {
      $(this).click(function() {
        var toggleable = $('#' + id);
        toggleable.toggle();
        $(this).toggleClass('toggler-active');
        return false;
      });
    }
    // Target does not exist, remove click handler.
    else {
      $(this).addClass('toggler-disabled');
      $(this).click(function() { return false; });
    }
    // Mark as processed.
    $(this).addClass('rubik-processed');
  });

    // If there's no active secondary tab, make the first one show.
  var activeli = $('.primary-tabs li.active .secondary-tabs li.active');
  if (activeli.length === 0) {
    $('.primary-tabs li.active .secondary-tabs li:first-child a').css('display', 'block');
  }
  
  $('.secondary-tabs li a, .secondary-tabs', context).bind('focus blur', function(){
    $(this).parents('.secondary-tabs').toggleClass('focused');
  });

  // Sticky sidebar functionality.
  var disableSticky = (settings.rubik !== undefined) ? settings.rubik.disable_sticky : false;
  if ($('#content .column-side .column-wrapper').length !== 0 ) {

    // Move fields to sidebar if it exists.
    $('.rubik_sidebar_field', context).once('rubik', function() {
      $('.column-side .column-wrapper').append($(this));
    });

    // Check if the sidebar should be made sticky.
    if (!disableSticky) {
      var rubikColumn = $('#content .column-side .column-wrapper', context);
      if (rubikColumn && rubikColumn.offset()) {
        var rubikStickySidebar = rubikColumn.offset().top;
        $(window).scroll(function() {
          if ($(window).scrollTop() > rubikStickySidebar) {
            rubikColumn.each(function() {
              $(this).addClass("fixed");
              $(this).width($(this).parent().width());
            });
          }
          else {
            rubikColumn.each(function() {
              $(this).removeClass("fixed");
              $(this).width($(this).parent().width());
            });
          }
        });
      }
    }

  }
  
  // Cache the primary tabs.
  var $primaryTabsWrap = $('.primary-tabs');
  if ($primaryTabsWrap.length) {
    var $primaryTabs = $primaryTabsWrap.find('> li');
    // Trigger adjusting function upon first page load.
    adjustPrimaryTabs();
    // Trigger adjusting function upon any screen resizing.
    $(window).resize(function() {
      adjustPrimaryTabs();
    });
  }

  function adjustPrimaryTabs() {
    // Get the position of whole element.
    var parentPosition = $primaryTabs.offset().top;
    // Complicated count.
    var count = [];
    var rowNumber = 1;
    // Remove remainings of other classes we attached.
    $primaryTabs.removeClass('last-row-link');
    $primaryTabs.removeClass('first-row-link');
    // Loop through and compare the position of each tab.
    $primaryTabs.each(function(index) {
      var $this = $(this);
      // New row.
      if (count[rowNumber] != $this.offset().top) {
        // Increase the count for this row.
        rowNumber++;
        count[rowNumber] = $this.offset().top;
        // Add "first" class to this element.
        $this.addClass('first-row-link');
        // Add "last" class to the previous element, if there is one.
        if ($this.prev('li').length) {
          $this.prev('li').addClass('last-row-link');
        }
      }
      // Add "last" class if this is the last element.
      if (index === ($primaryTabs.length - 1)) {
        $this.addClass('last-row-link');
      }
    });
  }

};
})(jQuery);
