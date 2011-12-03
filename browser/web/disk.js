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
	var done = eval(cef.plugin.search("[///index_status"));
	var drives = eval(cef.plugin.search("[///get_drives"));
	var offline = [];
	$.each(done, function (index, ele) { 
		if(!contain(ele.id,drives)){
			offline.push(ele);
		}
	});
	return offline;
}


function get_offline_dbs(){
	var dbs = eval(cef.plugin.search("[///cache_dbs"));
	dbs_name = dbs.map(function(ele){
		return ele.name;
	})
	var drives = eval(cef.plugin.search("[///get_drives"));
	$.each(drives, function (index, ele) { 
		var i = $.inArray(ele.serialNumber+".db", dbs_name);
		if(i!=-1) dbs_name.splice(i, 1);
	});
	return dbs_name;
}

function add_time_info(objs){
	var dbs = eval(cef.plugin.search("[///cache_dbs"));
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
	var done = eval(cef.plugin.search("[///index_status"));
	var drives = eval(cef.plugin.search("[///get_drives"));
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

function is_scan_finish_one(){
	var dd = eval(cef.plugin.search("[///index_status"));
	console.log(dd);
	return dd.length > 0;
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
	var done = eval(cef.plugin.search("[///index_status"));
	var drives = eval(cef.plugin.search("[///get_drives"));
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
	$("#select-hotkey").val(cef.plugin.hotkey);
	$("#dialog-set-hotkey").dialog({modal: true, width:600});
}
function set_hotkey(){
	cef.plugin.hotkey = $("#select-hotkey").val();
	if($("#select-hotkey").val()==cef.plugin.hotkey){
		$("#dialog-set-hotkey").dialog("close");
		show_info("快捷键设置成功！");
	}else{
		$("#dialog-set-hotkey").dialog("close");
		show_error("快捷键设置失败！");
	}
}

function show_export(){
	d = new Date();
	$("#export-filename").val("search-results-"+d.getFullYear()+
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
	cef.plugin.save($("#export-filename").val(),s);
	$("#dialog-export").dialog("close");
	show_info("查询结果导出成功！");
}

function rescan(td, i){
	clearTimeout(refreash_index_dialog_timeout);
	$(td.parentNode.cells(td.cellIndex-1)).html('<img src="images/spinner.gif">');
	cef.plugin.search("[///rescan"+i);
	refreash_index_dialog_timeout = setTimeout(refreash_index_status,3000);
}

function del_offline_db(td,i){
	$(td).html('<img src="images/spinner.gif">');
	function rescan0(){
		cef.plugin.search("[///del_offline_db"+i);
		$("#dialog-index-status").dialog("close");
		show_index_status();
	}	
	setTimeout(rescan0,10);
}

function offline_db(){
	$("#loading").css("visibility","visible");
	cef.plugin.offline = true;
	cef.plugin.search("[///load_offline_db");
	//判断离线DB是否加载完成
	setTimeout(refresh,2000);
}
function online_db(){
	cef.plugin.offline = false;
	setTimeout(refresh,10);
}

var mannual;//更新请求是手动的还是自动的。
function upgrade_req(up_url){
	var data = "upgrade[os]="+cef.gigaso.os+"&upgrade[cpu]="+cef.gigaso.cpu+"&upgrade[disk]="
		+cef.gigaso.disk+"&upgrade[ver]="+cef.gigaso.ver+"&upgrade[user]="+cef.gigaso.user;
	var jqxhr = $.ajax({
	  type: 'POST',
	  url: up_url+"/upgrades.js",
	  data: data,
	  dataType: "json"
	})
    .success(function(data, textStatus, jqXHR) { 
		console.log(data);
		if(data.status==1){
			cef.plugin.search("[///upgrade"+up_url+data.url+"?"+data.hash+"&"+data.version);
		}else{
			cef.plugin.search("[///upgrade_none");
		}
		setTimeout(show_upgrade_info,10);
	 })
    .error(function(jqXHR, textStatus, errorThrown) {
		console.log(up_url);
		if(up_url!=host_backup) upgrade_req(host_backup);
	});
}

function ver_new(){
	file = cef.plugin.search("[///upgrade_file");
	file.match(/PeaSearch-([\d\.]+)-x86.exe/i)[1]
}

function show_upgrade(){
	var jqxhr = $.ajax({
	  type: 'GET',
	  url: host+"/gigasos.js?version="+ver_new(),
	  dataType: "json"
	})
    .success(function(data, textStatus, jqXHR) { 
	    $('#upgrade_desc').html(data.desc);
		$("#old_version").html(cef.gigaso.ver);
		$("#new_version").html(ver_new());
		$("#dialog-upgrade").dialog({modal: true, width:600});
	 });
}

function do_upgrade(){
	$("#dialog-upgrade").dialog("close");
	file = cef.plugin.search("[///upgrade_file");
	if(file) cef.gigaso.shellDefault(file);
}

function show_upgrade_info(){
	if(mannual){
		switch(cef.plugin.search("[///upgrade_status")*1){
			case 0:  show_info("正在检查更新...");break;
			case 1:  show_info("已经更新到最新。");break;
			case 2:  show_info("安装更新...");break;
			case 3:  show_info("正在下载更新...");break;
			default:  show_info("暂时无法更新...");break;
		}
	}
}

function check_upgrade(by_hand){
   if(arguments.length==0) mannual=false;
   else mannual = by_hand;
   var update_status = cef.plugin.search("[///upgrade_status")*1;
   if(mannual || update_status==0){
   	upgrade_req(host_main);
   }else if(update_status==2){
   	show_upgrade();
   }
   //show_upgrade_info();
}

var first_grid=true;
var page_row=100;
var cur_page_end=200;
function file_grid(){
	$("#maintable").jqGrid({ data: files, 
						datatype: "local", 
						width : "1000",
						height: "auto",
						pager: false,
						colNames:['文件名','位置','大小','修改日期',''], 
						colModel:[ 
									{name:'icon_name', width:"40%",sortable:false },
									{name:'path', width:"38%",sortable:false},
									{name:'size', width:"7%", align:"right",sortable:false},
									{name:'time', width:"15%", align:"right",sortable:false},
									{name:'type', width:"0%", hidden:true,sortable:false}
								], 
						rowNum:cur_page_end,
						gridview: true,
						forceFit : true,
						scrollrows: false,
						scroll: false,
						multiselect: false,
						/*
							gridComplete : function() {
								var tm = jQuery("#maintable").jqGrid('getGridParam','totaltime');
								$("#dir").val("Render time: "+ tm+" ms ");
							},
						*/
					   onSelectRow: function(id,status){ 
						  idd=id*1+1; //id从1起，但是jggrow从2起
						  console.log(id+":"+idd);
						  if(status){
							if($("#maintable").jqGrid('getGridParam','selarrrow').length>1){
						      $(".jqgrow:nth-child("+idd+")").contextMenu('myMenu3', context_menu_obj3);
							}
						  }else{
						    $(".jqgrow:nth-child("+idd+")").contextMenu('myMenu1', context_menu_obj);
						  }
						  if($(".jqgrow:nth-child("+idd+") td")[1].title.substr(0,1)=="\\"){
						      $(".jqgrow:nth-child("+idd+")").contextMenu('',{});
						  }
					   },
						sortable: false});
	$(".ui-jqgrid-htable th").bind('click',function(event,t){
		if(this.id=="maintable_icon_name") var index=0;
		if(this.id=="maintable_path") var index=1;
		if(this.id=="maintable_size") var index=2;
		if(this.id=="maintable_time") var index=3;
		if(index==order_col){
			orderby(index,!order_desc);
		}else{
			orderby(index,1);
		}
	});
	first_grid=false;
	set_order_arrow(order_col+1,order_desc);
	grid_auto_width();
	grid_event();
}
function grid_start_id(){
	if(cur_page_end==200) return 0;
	else return cur_page_end-page_row;
}
function grid_event(){
	$("#maintable").setGridParam({multiselect: true});
	$(window).bind('resize',function(event){
		grid_auto_width();
	});
	$("#maintable tr:nth-child(even)").addClass("d");
	$("#maintable .jqgrow:not(.ped)").bind('keyup',function(e){
		console.log(e.keyCode);
		if(e.keyCode==13){
			dblclick_file($(".jqgrow", "#maintable")[this.id-1]);
		}else if(e.keyCode==46){
			delete_file($(".jqgrow", "#maintable")[this.id-1]);
		}else if(e.keyCode==67){
			copy_file($(".jqgrow", "#maintable")[this.id-1]);
		}else if(e.keyCode==88){
			cut_file($(".jqgrow", "#maintable")[this.id-1]);
		}
	});
	if(!cef.plugin.offline){
		$("#maintable .jqgrow:not(.ped)").contextMenu('myMenu1', context_menu_obj);
		$("#maintable .jqgrow:not(.ped) td:nth-child(1)").bind('dblclick',function(e){
			dblclick_file($(e.currentTarget).parent("tr"));
		});
		$.each($("#maintable .jqgrow:not(.ped) td:nth-child(1)"), function (index, ele) { 
			ele.addEventListener('dragstart', function(e){
				if(os_win()){
					get_file_path($(this).parent("tr"));
					//e.dataTransfer.setData('DownloadURL', "application/octet-stream:"+file+":"+path+file);
					e.dataTransfer.setData('URL',path+file);
				}else{
					drag_file($(e.currentTarget).parent("tr"));
				}
			}, false);
		});
		$("#maintable .jqgrow:not(.ped) td:nth-child(1)").attr("draggable","true");
		$("#maintable .jqgrow:not(.ped) td:nth-child(2)").bind('dblclick',function(e){
			dblclick_path($(e.currentTarget).parent("tr"));
		});
	}else{
		$("#maintable .jqgrow:not(.ped)").contextMenu('myMenu2', context_menu_obj);
	}
	highlight_timeout = setTimeout(highlight,1);
}
function grid_auto_width(){
	$("#maintable").jqGrid('setGridWidth',$("#tab-1").width());
}
function view_grid(){
	if(first_grid) file_grid();
	else{
		$("#maintable").setGridParam({multiselect: false});
		$("#maintable").clearGridData();
		$("#maintable").setGridParam({data: files}).trigger("reloadGrid");
		grid_event();
	}
}
function grid_add_files(){
	$("#maintable").setGridParam({multiselect: false});
	$("#maintable").addRowData(cur_page_end+1, files.slice(cur_page_end,cur_page_end+page_row), "last");
	$("#maintable tr:gt("+cur_page_end+")").each(function(idx,t){
		$(t).attr("id",cur_page_end+idx+1);
	});
	cur_page_end += page_row;
	grid_event();
}

function grid_selectd_files(){
	var selectd_file_ids = $("#maintable").jqGrid('getGridParam','selarrrow');
	var ret="";
	$.each(selectd_file_ids, function (index, ele) { 
		var file = files[ele-1];
		ret += file.path;
		ret += file.name;
		ret += "|";
	});
	return ret;
}
function grid_selectd_filenames(){
	var selectd_file_ids = $("#maintable").jqGrid('getGridParam','selarrrow');
	var ret="";
	$.each(selectd_file_ids, function (index, ele) { 
		var file = files[ele-1];
		ret += file.name;
		ret += "\n";
	});
	return ret;
}
function grid_batch_unselect(){
	$("#maintable .jqgrow").contextMenu('myMenu1', context_menu_obj);
	$("#maintable").resetSelection();
}
function has_row_selected(rowid){
	var selectd_file_ids = $("#maintable").jqGrid('getGridParam','selarrrow');
	var ret=false;
	$.each(selectd_file_ids, function (index, ele) { 
		if(ele==rowid+'') ret=true;
	});
	return ret;
}

