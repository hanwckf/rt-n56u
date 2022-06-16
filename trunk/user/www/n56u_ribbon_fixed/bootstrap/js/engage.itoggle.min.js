/* http://keith-wood.name/backgroundPos.html
 Background position animation for jQuery v1.0.1.
 Written by Keith Wood (kbwood{at}iinet.com.au) November 2010.
 Dual licensed under the GPL (http://dev.jquery.com/browser/trunk/jquery/GPL-LICENSE.txt) and
 MIT (http://dev.jquery.com/browser/trunk/jquery/MIT-LICENSE.txt) licenses.
 Please attribute the author if you use it. */
(function($){var g='bgPos';$.fx.step['backgroundPosition']=$.fx.step['background-position']=function(a){if(!a.set){var b=$(a.elem);var c=b.data(g);b.css('backgroundPosition',c);a.start=parseBackgroundPosition(c);a.end=parseBackgroundPosition($.fn.jquery>='1.6'?a.end:a.options.curAnim['backgroundPosition']||a.options.curAnim['background-position']);for(var i=0;i<a.end.length;i++){if(a.end[i][0]){a.end[i][1]=a.start[i][1]+(a.end[i][0]=='-='?-1:+1)*a.end[i][1]}}a.set=true}$(a.elem).css('background-position',((a.pos*(a.end[0][1]-a.start[0][1])+a.start[0][1])+a.end[0][2])+' '+((a.pos*(a.end[1][1]-a.start[1][1])+a.start[1][1])+a.end[1][2]))};function parseBackgroundPosition(c){var d={center:'50%',left:'0%',right:'100%',top:'0%',bottom:'100%'};var e=c.split(/ /);var f=function(a){var b=(d[e[a]]||e[a]||'50%').match(/^([+-]=)?([+-]?\d+(\.\d*)?)(.*)$/);e[a]=[b[1],parseFloat(b[2]),b[4]||'px']};if(e.length==1&&$.inArray(e[0],['top','bottom'])>-1){e[1]=e[0];e[0]='50%'}f(0);f(1);return e}$.fn.animate=function(e){return function(a,b,c,d){if(a['backgroundPosition']||a['background-position']){this.data(g,this.css('backgroundPosition')||'center')}return e.apply(this,[a,b,c,d])}}($.fn.animate)})(jQuery);

/*---------------
 * jQuery iToggle Plugin by Engage Interactive
 * Examples and documentation at: http://labs.engageinteractive.co.uk/itoggle/
 * Copyright (c) 2009 Engage Interactive
 * Version: 1.0 (10-JUN-2009)
 * Dual licensed under the MIT and GPL licenses:
 * http://www.opensource.org/licenses/mit-license.php
 * http://www.gnu.org/licenses/gpl.html
 * Requires: jQuery v1.3 or later
 ---------------*/
(function(a){a.fn.iToggle=function(g){function f(b,d){!0==b?"radio"==c.type?a("label[for="+d+"]").addClass("ilabel_radio"):a("label[for="+d+"]").addClass("ilabel"):a("label[for="+d+"]").remove()}function e(b,d){c.onClick.call(b);h=b.innerHeight();t=b.attr("for");b.hasClass("iTon")?(c.onClickOff.call(b),b.animate({backgroundPosition:"100% -"+h+"px"},c.speed,c.easing,function(){b.removeClass("iTon").addClass("iToff");clickEnabled=!0;c.onSlide.call(this);c.onSlideOff.call(this)}),a("input#"+t).removeAttr("checked")): (c.onClickOn.call(b),b.animate({backgroundPosition:"0% -"+h+"px"},c.speed,c.easing,function(){b.removeClass("iToff").addClass("iTon");clickEnabled=!0;c.onSlide.call(this);c.onSlideOn.call(this)}),a("input#"+t).attr("checked","checked"));!0==d&&(name=a("#"+t).attr("name"),e(b.siblings("label[for]")))}clickEnabled=!0;var c=a.extend({},{type:"checkbox",keepLabel:!0,easing:!1,speed:200,onClick:function(){},onClickOn:function(){},onClickOff:function(){},onSlide:function(){},onSlideOn:function(){},onSlideOff:function(){}}, g);this.each(function(){var b=a(this);if("INPUT"==b.attr("tagName")){var d=b.attr("id");f(c.keepLabel,d);b.addClass("iT_checkbox").before('<label class="itoggle" for="'+d+'"><span></span></label>');b.attr("checked")?b.prev("label").addClass("iTon"):b.prev("label").addClass("iToff");b.children("input:first").is(":disabled")?a(this).prev("label").addClass("disabled"):a(this).prev("label").removeClass("disabled")}else b.children("input:"+c.type).each(function(){var d=a(this).attr("id");f(c.keepLabel, d);a(this).addClass("iT_checkbox").before('<label class="itoggle" for="'+d+'"><span></span></label>');a(this).attr("checked")?a(this).prev("label").addClass("iTon"):a(this).prev("label").addClass("iToff");"radio"==c.type&&a(this).prev("label").addClass("iT_radio");b.children("input:first").is(":disabled")?a(this).prev("label").addClass("disabled"):a(this).prev("label").removeClass("disabled")})});a("label.itoggle").click(function(){if(a("#"+a(this).attr("for")).is(":disabled"))return!1;!0==clickEnabled&& (clickEnabled=!1,a(this).hasClass("iT_radio")?a(this).hasClass("iTon")?clickEnabled=!0:e(a(this),!0):e(a(this)));return!1});a("label.ilabel").click(function(){if(a("#"+a(this).attr("for")).is(":disabled")||a(this).hasClass("disabled"))return!1;!0==clickEnabled&&(clickEnabled=!1,e(a(this).next("label.itoggle")));return!1})}})(jQuery);

(function($j){
    $j.fn.iClickable = function(flag) {

        this.each(function(){
            var $this = $j(this);

            if(typeof $this === 'object')
            {
                if(flag === false || flag === 0)
                {
                    $this.children('.itoggle').addClass('disabled');
                    $this.children('input:first').attr('disabled', 'disabled');
                }
                else if(flag === true || flag === 1)
                {
                    $this.children('.itoggle').removeClass('disabled');
                    $this.children('input:first').removeAttr('disabled');
                }
            }
        });

        return this;
    };
})(jQuery);

(function($j){
    $j.fn.iState = function(new_state) {

        this.each(function(){
            $object = $j(this);

            var id = "#"+$object.attr('id');

            var on_off_position = ['100% -27px', '0% -27px'];
            var cur_enabled = $object.children('input:first').is(":checked");

            var $fakeOn = $object.parents('.main_itoggle').next().children('input:nth-child(1)');
            var $fakeOff = $object.parents('.main_itoggle').next().children('input:nth-child(2)');

            new_state = new_state > 0;

            if(new_state != cur_enabled)
            {
                $j(id+" label.itoggle").css("background-position", on_off_position[Number(new_state)]);

                if(new_state)
                {
                    $object.children('input:first').val($fakeOn.val());
                    $object.children('input:first').attr('checked', 'checked');
                    $fakeOn.attr('checked', 'checked');
                    $fakeOff.removeAttr('checked');
                }
                else
                {
                    $object.children('input:first').val($fakeOff.val());
                    $object.children('input:first').removeAttr('checked');
                    $fakeOn.removeAttr('checked');
                    $fakeOff.attr('checked', 'checked');
                }
            }
        });

        return this;
    };
})(jQuery);
