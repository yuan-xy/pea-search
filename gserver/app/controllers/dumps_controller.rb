class DumpsController < ApplicationController
  # GET /dumps
  # GET /dumps.xml
  def index
    @dumps = Dump.all

    respond_to do |format|
      format.html # index.html.erb
      format.xml  { render :xml => @dumps }
    end
  end

  # GET /dumps/1
  # GET /dumps/1.xml
  def show
    @dump = Dump.find(params[:id])

    respond_to do |format|
      format.html # show.html.erb
      format.xml  { render :xml => @dump }
    end
  end

  # GET /dumps/new
  # GET /dumps/new.xml
  def new
    @dump = Dump.new

    respond_to do |format|
      format.html # new.html.erb
      format.xml  { render :xml => @dump }
    end
  end

  # GET /dumps/1/edit
  def edit
    @dump = Dump.find(params[:id])
  end

  # POST /dumps
  # POST /dumps.xml
  def create
    @dump = Dump.new(params[:dump])
    name =  params[:upload_file_minidump].original_filename  
    directory = "public/dumps"  
    path = File.join(directory, name)  
    File.open(path, "wb") { |f| f.write(params[:upload_file_minidump].read) }  
    @dump.file=name;
    @dump.ip=request.ip;
    respond_to do |format|
      if @dump.save
        format.html { redirect_to(@dump, :notice => 'Dump was successfully created.') }
        format.xml  { render :xml => @dump, :status => :created, :location => @dump }
      else
        format.html { render :action => "new" }
        format.xml  { render :xml => @dump.errors, :status => :unprocessable_entity }
      end
    end
  end

  # PUT /dumps/1
  # PUT /dumps/1.xml
  def update
    @dump = Dump.find(params[:id])

    respond_to do |format|
      if @dump.update_attributes(params[:dump])
        format.html { redirect_to(@dump, :notice => 'Dump was successfully updated.') }
        format.xml  { head :ok }
      else
        format.html { render :action => "edit" }
        format.xml  { render :xml => @dump.errors, :status => :unprocessable_entity }
      end
    end
  end

  # DELETE /dumps/1
  # DELETE /dumps/1.xml
  def destroy
    @dump = Dump.find(params[:id])
    @dump.destroy

    respond_to do |format|
      format.html { redirect_to(dumps_url) }
      format.xml  { head :ok }
    end
  end
end
