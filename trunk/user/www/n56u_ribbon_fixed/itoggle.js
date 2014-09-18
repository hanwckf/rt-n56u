function init_itoggle(id,func)
{
    var obj_f = $j('#'+id+'_fake');
    var obj_0 = $j('#'+id+'_0');
    var obj_1 = $j('#'+id+'_1');

    $j('#'+id+'_on_of').iToggle({
        easing: 'linear',
        speed: 70,
        onClickOn: function(){
            obj_f.attr("checked","checked").attr("value",1);
            obj_1.attr("checked","checked");
            obj_0.removeAttr("checked");
            if (typeof(func) === 'function')
                func();
        },
        onClickOff: function(){
            obj_f.removeAttr("checked").attr("value",0);
            obj_0.attr("checked","checked");
            obj_1.removeAttr("checked");
            if (typeof(func) === 'function')
                func();
        }
    });
    $j("#"+id+"_on_of label.itoggle").css("background-position", $j("input#"+id+"_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
}
