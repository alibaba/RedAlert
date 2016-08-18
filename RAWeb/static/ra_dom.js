Date.prototype.format = function(format)
{
    var o = {
        "M+" : this.getMonth()+1, //month
        "d+" : this.getDate(),    //day
        "h+" : this.getHours(),   //hour
        "m+" : this.getMinutes(), //minute
        "s+" : this.getSeconds(), //second
        "q+" : Math.floor((this.getMonth()+3)/3),  //quarter
        "S" : this.getMilliseconds() //millisecond
    }

    if(/(y+)/.test(format)) format=format.replace(RegExp.$1,
                                                  (this.getFullYear()+"").substr(4 - RegExp.$1.length));
    for(var k in o)if(new RegExp("("+ k +")").test(format))
        format = format.replace(RegExp.$1,
                                RegExp.$1.length==1 ? o[k] :
                                ("00"+ o[k]).substr((""+ o[k]).length));
    return format;
}

function set_cookie(k, v, seconds)
{
    k = encodeURIComponent(k);
    var now = new Date();
    seconds = seconds || 86400;
    expire = new Date(now.getTime() + seconds*1000);
    document.cookie=k+"="+v+"; expires="+expire.toGMTString()+"; path=/";
}

function del_cookie(k)
{
    set_cookie(k, "", 0);
}

function get_cookie(key)
{
    var result;
    return (result = new RegExp('(?:^|; )' + encodeURIComponent(key) + '=([^;]*)').exec(document.cookie)) ? (result[1]) : null;
}

function sign_in_onclick(evt)
{
    evt.preventDefault();
    var username = $.trim($('#user_name').val());
    if (username.length == 0) {
        alert("用户名不得为空");
    }
    
    set_cookie("username", username);
    toggle_sign_in(username);
}

function acquire_lock_onclick(evt)
{
    evt.preventDefault();
}

function sign_out_onclick(evt)
{
    evt.preventDefault();
    $('#user_name').val("");
    del_cookie("username");
    toggle_sign_in(null);
}

function toggle_sign_in(ra_user)
{
    // $('#user_container .toggle_display').hide();
    // if (null == ra_user || 0 == ra_user.length) {
    //     $('#sign_in_form_group').show();
    //     $('#sign_in').show();
    // }
    // else {
    //     $('#sign_out').show();
    //     $('#user_name_display').text(ra_user);
    //     $('#user_display').show();
    // }

    $('#user_name_display').text(ra_user);
    $('#user_display').show();
}

$(function()
{
    // nav bar events handler
    // $('#sign_in').click(sign_in_onclick);
    // $('#acquire_lock').click(acquire_lock_onclick);
    // $('#sign_out').click(sign_out_onclick);
    // $('#user_name_display').text('');
    
    var ra_user = get_cookie("username");

    toggle_sign_in(ra_user);
    
    var tooltip_opt = {
        'html': true
    };
    $('.ra_tooltip').tooltip(tooltip_opt);
});
