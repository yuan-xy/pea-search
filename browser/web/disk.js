function drive_type_name(type){
	if (type==2) return "优盘"
	if (type==3) return "外置硬盘"
	if (type==4) return "网络磁盘"
	if (type==5) return "光盘"
	return ""
}

function get_drive_info(id){
	var d_infos = eval(plugin.search("[///index_status"));
	for(var f_index=0; f_index<d_infos.length; f_index++){
		if(d_infos[f_index].id == id) return d_infos[f_index];
	}
}

function gen_offline_dir_name(file){
	var id = file.path.substring(0,2);
	var dir = file.path.substring(2, file.path.length);
	var info = get_drive_info(id);
	var disk_name = (info.volumeName ? info.volumeName : '未命名'+(id-25));
	file.path = "["+drive_type_name(info.type)+":"+disk_name+"]"+dir;
}