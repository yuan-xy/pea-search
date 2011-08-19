function drive_type_name(type){
	if (type==0) return "未知"
	if (type==1) return "无效"
	if (type==2) return "U盘"
	if (type==3) return "硬盘"
	if (type==4) return "网络磁盘"
	if (type==5) return "光盘"
	if (type==6) return "内存盘"
	return ""
}

function contain(id, obj){
		var ret=false;
		$.each(obj, function (index, ele) { 
			if(ele.id == id){
				ret = true;
			}
		});
		return ret;
}

function get_drive_info(id, d_infos){
	for(var f_index=0; f_index<d_infos.length; f_index++){
		if(d_infos[f_index].id == id) return d_infos[f_index];
	}
}

function gen_offline_dir_name(file, d_infos){
	var id = file.path.substring(0,2);
	var dir = file.path.substring(2, file.path.length);
	var info = get_drive_info(id, d_infos);
	var disk_name = (info.volumeName ? info.volumeName : '未命名'+(id-25));
	file.path = "["+drive_type_name(info.type)+":"+disk_name+"]"+dir;
}

function get_loaded_offline_dbs(){
	var done = eval(plugin.search("[///index_status"));
	var drives = eval(plugin.search("[///get_drives"));
	var offline = [];
	$.each(done, function (index, ele) { 
		if(!contain(ele.id,drives)){
			offline.push(ele);
		}
	});
	return offline;
}


function get_offline_dbs(){
	var dbs = eval(plugin.search("[///cache_dbs"));
	var drives = eval(plugin.search("[///get_drives"));
	$.each(drives, function (index, ele) { 
		var i = $.inArray(ele.serialNumber+".db", dbs);
		if(i!=-1) dbs.splice(i, 1);
	});
	return dbs;
}

//判断是否有离线db
function has_offline_dbs(){
	return get_offline_dbs().length>0;
}

function todo_dirves(){
	var done = eval(plugin.search("[///index_status"));
	var drives = eval(plugin.search("[///get_drives"));
	done_sn = done.map(function(ele){
		return ele.serialNumber;
	})
	drive_sn = drives.map(function(ele){
		return ele.serialNumber;
	})
	$.each(done_sn, function (index, ele) { 
		var i = $.inArray(ele, drive_sn);
		if(i!=-1) drive_sn.splice(i, 1);
	});
	var ret = [];
	$.each(drive_sn, function (index, ele) { 
		if(ele.length>0 && ele!="0") ret.push(ele);
	});
	return ret;
}

function is_scan_finished(){
	return todo_dirves().length == 0
}

function scan_img(){
	if(is_scan_finished()){
		$("#scanning").css("visibility","hidden");
		return;
	}
	$("#scanning").css("visibility","visible");
	setTimeout(scan_img, 5000);
}

function show_index_status(){
	var done = eval(plugin.search("[///index_status"));
	var drives = eval(plugin.search("[///get_drives"));
	$.each(drives, function (index, ele) { 
		if(!ele.volumeName) ele.volumeName="未命名";
		ele.panfu = String.fromCharCode(65+ele.id*1);
		ele.typename = drive_type_name(ele.type);
		if(contain(ele.id,done)){
			ele.indexed = "images/tick.png";
		}else{
			if(ele.serialNumber.length>1) ele.indexed = "images/spinner.gif";
			else  ele.indexed = "";
		}
	});
	var offline = get_loaded_offline_dbs();
	$.each(offline, function (index, ele) { 
		ele.indexed = "离线";
		ele.typename = drive_type_name(ele.type);
	});
	$("#index_status_result").setTemplateElement("index_status_template");
	$("#index_status_result").processTemplate(drives);
	$("#offline-index_status_result").setTemplateElement("offline_index_status_template");
	$("#offline-index_status_result").processTemplate(offline);
	$("#dialog-index-status").dialog({modal: true, width:600});
}

function show_hotkey(){
	$("#select-hotkey").val(plugin.hotkey);
	$("#dialog-set-hotkey").dialog({modal: true});
}
function set_hotkey(){
	plugin.hotkey = $("#select-hotkey").val();
	$("#dialog-set-hotkey").dialog("close");
	show_info("快捷键设置成功！");
}
