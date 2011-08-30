class UpgradesController < ApplicationController
  before_filter :admin_authorize, :except => :create

  # GET /upgrades
  # GET /upgrades.xml
  def index
    @upgrades = Upgrade.paginate(:page => params[:page], :per_page =>20)


    respond_to do |format|
      format.html # index.html.erb
      format.xml  { render :xml => @upgrades }
    end
  end

  # GET /upgrades/1
  # GET /upgrades/1.xml
  def show
    @upgrade = Upgrade.find(params[:id])

    respond_to do |format|
      format.html # show.html.erb
      format.xml  { render :xml => @upgrade }
    end
  end

  # GET /upgrades/new
  # GET /upgrades/new.xml
  def new
    @upgrade = Upgrade.new

    respond_to do |format|
      format.html # new.html.erb
      format.xml  { render :xml => @upgrade }
    end
  end

  # GET /upgrades/1/edit
  def edit
    @upgrade = Upgrade.find(params[:id])
  end

  # POST /upgrades
  # POST /upgrades.xml
  def create
    @upgrade = Upgrade.new(params[:upgrade])
    @upgrade.ip = request.ip;
    respond_to do |format|
      if @upgrade.save
        format.html { render :text => "ok" }
        format.xml  { render :xml => @upgrade, :status => :created, :location => @upgrade }
        format.js
      else
        format.html { render :text => "error" }
        format.xml  { render :xml => @upgrade.errors, :status => :unprocessable_entity }
      end
    end
  end

  # PUT /upgrades/1
  # PUT /upgrades/1.xml
  def update
    @upgrade = Upgrade.find(params[:id])

    respond_to do |format|
      if @upgrade.update_attributes(params[:upgrade])
        format.html { redirect_to(@upgrade, :notice => 'Upgrade was successfully updated.') }
        format.xml  { head :ok }
      else
        format.html { render :action => "edit" }
        format.xml  { render :xml => @upgrade.errors, :status => :unprocessable_entity }
      end
    end
  end

  # DELETE /upgrades/1
  # DELETE /upgrades/1.xml
  def destroy
    @upgrade = Upgrade.find(params[:id])
    @upgrade.destroy

    respond_to do |format|
      format.html { redirect_to(upgrades_url) }
      format.xml  { head :ok }
    end
  end
end
