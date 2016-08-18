(function()
{
    // Establish the root object, `window` in the browser, or `exports` on the server.
    var root = this;
    var ra_api = function() {};
    root.ra_api = ra_api;
    ra_api.SUCC = 0;
    ra_api.NO_CONFIG = 409;
    ra_api.NO_LOGIN = 110;
    ra_api.WR_JSON_ERROR = 104;
    ra_api.RD_JSON_ERROR = 105;
    ra_api.LOCK_BY_OTHERS = 111;
    
    ra_api.setup = function(api_server, timeout)
    {
        ra_api._api_server = api_server || "http://localhost";
        ra_api._timeout = timeout || 3000;
        return ra_api;
    };
    
    ra_api.set_callbacks = function(succ_cb, fail_cb, complete_cb)
    {
        ra_api._succ_cb = succ_cb;
        ra_api._fail_cb = fail_cb;
        ra_api._complete_cb = complete_cb;
        return ra_api;
    }
    
    ra_api._get = function(path, param_json)
    {
        var url = ra_api._api_server + path;
        //var data = {"p": JSON.stringify(param_json)};
        $.ajax({
            "cache":false,
            "data": param_json,
            "dataType": "json",
            "error": ra_api._fail_cb,
            "success": ra_api._succ_cb,
            "complete": ra_api._complete_cb,
            "timeout": ra_api._timeout,
            "type": "GET",
            "url": url
        });
    };
    
    ra_api.get_policies = function(group, page_no, page_size)
    {
        var param_json = {
            "group": group,
            "page_no": page_no,
            "page_size": page_size
        };
        ra_api._get("/api/get_policies", param_json);
        return ra_api;
    };
    
    ra_api.get_policy = function(id)
    {
        var param_json = {
            "id": id
        };
        
        ra_api._get("/api/get_policy", param_json);
        return ra_api;
    };
    
    ra_api.add_policy = function(policy_opt)
    {
        ra_api._get("/api/add_policy", policy_opt);
        return ra_api;
    };
    
    ra_api.update_policy = function(policy_id, policy_opt)
    {
        policy_opt['id'] = policy_id;
        ra_api._get("/api/update_policy", policy_opt);
        return ra_api;
    };
    ra_api.cancel_policy = function(id)
    {
        var param_json = {
            "id": id
        };
        
        ra_api._get("/api/cancel_policy", param_json);
        return ra_api;
    };

    ra_api.invalid_policy = function(id)
    {
        var param_json = {
            "id": id
        };
        
        ra_api._get("/api/invalid_policy", param_json);
        return ra_api;
    };

    ra_api.valid_policy = function(id)
    {
        var param_json = {
            "id": id
        };
        
        ra_api._get("/api/valid_policy", param_json);
        return ra_api;
    };

    ra_api.update_group = function(oldGroup, newGroup, ids)
    {
        var param_json = {
            "oldGroup": oldGroup,
            "newGroup": newGroup,
            "ids": ids
        };

        ra_api._get("/api/group_move", param_json);
        return ra_api;
    }
    
    ra_api.get_groups = function()
    {
        ra_api._get("/api/get_groups", {});
        return ra_api;
    }
    
    ra_api.delete_policy = function(ids)
    {
        var param_json = {
            "ids": ids.join('_')
        };
        
        ra_api._get("/api/delete_policies", param_json);
        return ra_api;
    };
    
    ra_api.search_policy = function(group, metric_name)
    {
        var param_json = {
            "group": group,
            "query": metric_name
        };
        ra_api._get("/api/search_metric", param_json);
        return ra_api;
    };
    
    ra_api.add_amonitor = function(addr, name, fetch_type, optionString)
    {
        var param_json = {
            "address": addr,
            "service": name,
	    "fetcherType": fetch_type,
	    "optionString": optionString,
            "type": "dataSource"
        };
        
        ra_api._get("/sapi/add_item", param_json);
        return ra_api;
    };
    
    ra_api.remove_amonitor = function(ids)
    {
        var param_json = {
            "ids": ids.join("_"),
            "type": "dataSource"
        };
        
        ra_api._get("/sapi/del_item", param_json);
        return ra_api;
    };
    
    ra_api.get_amonitors = function()
    {
        var param_json = {
            "page_no": 1,
            "page_size": 1024,
            "type": "dataSource"
        };
        ra_api._get("/sapi/get_items", param_json);
        return ra_api;
    };
    
    ra_api.get_red_alerts = function()
    {
        var param_json = {
            "page_no": 1,
            "page_size": 1024,
            "type": "redAlert"
        };
        ra_api._get("/sapi/get_items", param_json);
        return ra_api;
    };
    
    ra_api.remove_red_alert = function(ids)
    {
        var param_json = {
            "ids": ids.join("_"),
            "type": "redAlert"
        };
        
        ra_api._get("/sapi/del_item", param_json);
        return ra_api;
    }
    
    ra_api.add_red_alert = function(id, addr, name, weight)
    {
        var param_json = {
            "type": "redAlert",
            "address": addr,
            "service": name,
            "weight": weight,
            "id": id
        };
        
        ra_api._get("/sapi/add_item", param_json);
        return ra_api;
    };
    
    ra_api.get_trend_param = function()
    {
        ra_api._get("/sapi/get_trendFactor", {});
        return ra_api;
    };
    
    ra_api.update_trend_param = function(smoothing_factor, trend_factor)
    {
        var param_json = {
            "smoothingFactor": smoothing_factor,
            "trendFactor": trend_factor
        };
        ra_api._get("/sapi/set_trendFactor", param_json);
        return ra_api;
    };
    
    ra_api.get_shield = function(page_no, page_size)
    {
        var param_json = {
            "page_no": page_no,
            "page_size": page_size,
            "type": "shield"
        };
        
        ra_api._get("/sapi/get_items", param_json);
    };
    
    ra_api.add_shield = function(group_name, metric_name, host_list, end_time)
    {
        var param_json = {
            "metric": metric_name,
            "host": host_list,
            "endTime": end_time,
            "group": group_name,
            "type": "shield"
        };
        
        ra_api._get("/sapi/add_item", param_json);
    };
    
    ra_api.cancel_item = function(id, type)
    {
        var param_json = {
            "id": id,
            "type": type
        };
        ra_api._get("/sapi/cancel_item", param_json);
    };

    ra_api.remove_shield = function(ids)
    {
        var param_json = {
            "ids": ids.join("_"),
            "type": "shield"
        };
        ra_api._get("/sapi/del_item", param_json);
    };
    
    ra_api.update_shield = function(id, group_name, metric_name, end_time)
    {
        var param_json = {
            "id": id,
            "metric": metric_name,
            "endTime": end_time,
            "group": group_name,
            "type": "shield"
        };
        
        ra_api._get("/sapi/update_item", param_json);
    };
    
    ra_api.start_service = function(recover)
    {
        var param_json = {
            "recover": recover? "yes": "no"
        };
        ra_api._get("/admin/start_service", param_json);
    };
    
    ra_api.stop_service = function()
    {
        ra_api._get("/admin/stop_service", {});
    };
    
    ra_api.revert_service = function(ver)
    {
        ra_api._get("/admin/reload", {"version": ver});
    };
    
    ra_api.deploy_config = function()
    {
        ra_api._get("/admin/deploy", {});
    };

    ra_api.get_diff = function()
    {
        ra_api._get("/admin/diff", {});
        return ra_api;
    };
    
    ra_api.reload_config = function()
    {
        ra_api._get("/admin/reload_latest", {});
    };

    ra_api.recover_config = function()
    {
        ra_api._get("/admin/recover", {});
    };
    
    ra_api.get_service_status = function()
    {
        ra_api._get("/admin/get_status", {});
    };
    
    ra_api.refresh_metrics = function()
    {
        ra_api._get("/admin/refresh_metrics", {});
    };
    
    ra_api.get_amonitor_info = function(spec, metric_path)
    {
        var param_json = {"spec":spec};
        if (metric_path.length >= 1) {
            param_json["service"] = metric_path[0];
        }
        
        if (metric_path.length >= 2) {
            param_json["nodepath"] = metric_path.slice(1).join(".");
        }
        ra_api._get("/api/fetch_metrics", param_json);
    };
    
    ra_api.alert_on_errno = function(rsp, ctx)
    {
        var errno = rsp['ret'];
        var errmsg = rsp['msg'];
        switch (errno) {
        case ra_api.SUCC:
            return false;
        case ra_api.NO_LOGIN:
            alert("请先登入再操作");
            break;
        case ra_api.RD_JSON_ERROR:
            alert("读取配置出错，文件可能已损坏，可考虑回滚配置("+errmsg+")");
            break;
        case ra_api.WR_JSON_ERROR:
            alert("写入配置出错，可能磁盘已满("+errmsg+")");
            break;
        case ra_api.NO_CONFIG:
            break;
        case ra_api.LOCK_BY_OTHERS:
            alert("他人正在修改，请勿操作！("+errmsg+")");
            break;
        default:
            alert(ctx+" [errno:"+errno+" msg:"+errmsg+"]")
            break;
        }
        
        return true;
    };
})();
