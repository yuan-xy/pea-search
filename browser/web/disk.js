var refreash_index_dialog_timeout;

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
	if(id>=26 && id<26*3){
		var dir = file.path.substring(2, file.path.length);
		var info = get_drive_info(id, d_infos);
		if(info.volumeName) disk_name = info.volumeName;
		else disk_name = '未命名'+(id-25);
		file.path = "["+drive_type_name(info.type)+":"+disk_name+"]"+dir;
	}
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
	dbs_name = dbs.map(function(ele){
		return ele.name;
	})
	var drives = eval(plugin.search("[///get_drives"));
	$.each(drives, function (index, ele) { 
		var i = $.inArray(ele.serialNumber+".db", dbs_name);
		if(i!=-1) dbs_name.splice(i, 1);
	});
	return dbs_name;
}

function add_time_info(objs){
	var dbs = eval(plugin.search("[///cache_dbs"));
	dbs_name = dbs.map(function(ele){
		return ele.name;
	})
	$.each(objs, function (index, ele) { 
		var i = $.inArray(ele.serialNumber+".db", dbs_name);
		if(i!=-1){
			d = new Date(dbs[i].time*1000);
			ele.time = d.getFullYear()+
		"-"+(d.getMonth()+1)+"-"+d.getDate()+ " "+d.getHours()+":"+d.getMinutes();
		}
	});
}

function parse_space(objs){
	$.each(objs, function (index, ele) {
		if(ele.totalMB>9000){
			ele.totalMB = Math.round(ele.totalMB/1000) + "GB";
			ele.totalFreeMB = Math.round(ele.totalFreeMB/1000) + "GB";
		} else if(ele.totalMB>900){
			ele.totalMB = Math.round(ele.totalMB/100)/10 + "GB";
			ele.totalFreeMB = Math.round(ele.totalFreeMB/100)/10 + "GB";
		}else{
			ele.totalMB = Math.round(ele.totalMB/10)*10 + "MB";
			ele.totalFreeMB = Math.round(ele.totalFreeMB/10)*10 + "MB";
		}
	});
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
		if(!ele.fsName) ele.display="display:none";
	});
	var offline = get_loaded_offline_dbs();
	$.each(offline, function (index, ele) { 
		ele.indexed = "离线";
		ele.typename = drive_type_name(ele.type);
		if(!ele.volumeName) ele.volumeName = '未命名 ('+(ele.id-25)+')';
	});
	$("#index_status_result").setTemplateElement("index_status_template");
	add_time_info(drives);
	parse_space(drives);
	$("#index_status_result").processTemplate(drives);
	$("#offline-index_status_result").setTemplateElement("offline_index_status_template");
	add_time_info(offline);
	parse_space(offline);
	$("#offline-index_status_result").processTemplate(offline);
	$("#dialog-index-status").dialog({modal: true, width:600});
}

function refreash_index_status(){
	if($("#dialog-index-status").parent().css("display")=="block"){
		show_index_status();
		refreash_index_dialog_timeout = setTimeout(refreash_index_status,1000);
	}
}

function show_hotkey(){
	$("#select-hotkey").val(plugin.hotkey);
	$("#dialog-set-hotkey").dialog({modal: true, width:600});
}
function set_hotkey(){
	plugin.hotkey = $("#select-hotkey").val();
	$("#dialog-set-hotkey").dialog("close");
	show_info("快捷键设置成功！");
}

function show_export(){
	d = new Date();
	$("#export-filename").val("c:\\gigaso-"+d.getFullYear()+
		"-"+(d.getMonth()+1)+"-"+d.getDate() +"'"+d.getSeconds() +".txt");
	$("#dialog-export").dialog({modal: true, width:600});
}
function do_export(){
	var etype = $("#select-export").val();
	var s = "";
	$.each(files, function (index, ele) { 
		if(ele.path2) s += ele.path2;
		else  s += ele.path;
		if(ele.name2) s += ele.name2;
		else s += ele.name;
		if(etype=="1"){
			s += "\t,"+ele.size+"\t,"+ele.time;
		}
		s += "\r\n";
	});
	plugin.save($("#export-filename").val(),s);
	$("#dialog-export").dialog("close");
	show_info("查询结果导出成功！");
}

function rescan(td, i){
	clearTimeout(refreash_index_dialog_timeout);
	$(td.parentNode.cells(td.cellIndex-1)).html('<img src="images/spinner.gif">');
	plugin.search("[///rescan"+i);
	refreash_index_dialog_timeout = setTimeout(refreash_index_status,3000);
}

function del_offline_db(td,i){
	$(td).html('<img src="images/spinner.gif">');
	function rescan0(){
		plugin.search("[///del_offline_db"+i);
		$("#dialog-index-status").dialog("close");
		show_index_status();
	}	
	setTimeout(rescan0,10);
}

function offline_db(){
	$("#loading").css("visibility","visible");
	plugin.offline = true;
	plugin.search("[///load_offline_db");
	//判断离线DB是否加载完成
	setTimeout(refresh,2000);
}
function online_db(){
	plugin.offline = false;
	setTimeout(refresh,10);
}