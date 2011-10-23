class GigasosController < ApplicationController
  before_filter :admin_authorize, :except => :index
  # GET /gigasos
  # GET /gigasos.xml
  def index
    @gigasos = Gigaso.paginate(:conditions => genCondition, :order => genOrder, :page => params[:page], :per_page =>20)

    respond_to do |format|
      format.html # index.html.erb
      format.js
      format.xml  { render :xml => @gigasos }
    end
  end

  # GET /gigasos/1
  # GET /gigasos/1.xml
  def show
    @gigaso = Gigaso.find(params[:id])

    respond_to do |format|
      format.html # show.html.erb
      format.xml  { render :xml => @gigaso }
    end
  end

  # GET /gigasos/new
  # GET /gigasos/new.xml
  def new
    @gigaso = Gigaso.new

    respond_to do |format|
      format.html # new.html.erb
      format.xml  { render :xml => @gigaso }
    end
  end

  # GET /gigasos/1/edit
  def edit
    @gigaso = Gigaso.find(params[:id])
  end

  # POST /gigasos
  # POST /gigasos.xml
  def create
    @gigaso = Gigaso.new(params[:gigaso])

    respond_to do |format|
      if @gigaso.save
        `ln -s #{@gigaso.path} /home/dooo/gserver/public/#{@gigaso.filename}`
        format.html { redirect_to(@gigaso, :notice => 'Gigaso was successfully created.') }
        format.xml  { render :xml => @gigaso, :status => :created, :location => @gigaso }
      else
        format.html { render :action => "new" }
        format.xml  { render :xml => @gigaso.errors, :status => :unprocessable_entity }
      end
    end
  end

  # PUT /gigasos/1
  # PUT /gigasos/1.xml
  def update
    @gigaso = Gigaso.find(params[:id])

    respond_to do |format|
      if @gigaso.update_attributes(params[:gigaso])
        format.html { redirect_to(@gigaso, :notice => 'Gigaso was successfully updated.') }
        format.xml  { head :ok }
      else
        format.html { render :action => "edit" }
        format.xml  { render :xml => @gigaso.errors, :status => :unprocessable_entity }
      end
    end
  end

  # DELETE /gigasos/1
  # DELETE /gigasos/1.xml
  def destroy
    @gigaso = Gigaso.find(params[:id])
    @gigaso.destroy

    respond_to do |format|
      format.html { redirect_to(gigasos_url) }
      format.xml  { head :ok }
    end
  end

   private
  def genCondition
    s = ""
    ad = ""
    h = Hash.new
    if has_value params[:version]
      s << ad
      s << " version = :version"
      h[:version] = "#{params[:version]}"
      ad = " and"
    end
    [s, h]
  end


  def genOrder
    s =" id desc"
  end


end
