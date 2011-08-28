require 'test_helper'

class UpgradesControllerTest < ActionController::TestCase
  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:upgrades)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create upgrade" do
    assert_difference('Upgrade.count') do
      post :create, :upgrade => { }
    end

    assert_redirected_to upgrade_path(assigns(:upgrade))
  end

  test "should show upgrade" do
    get :show, :id => upgrades(:one).to_param
    assert_response :success
  end

  test "should get edit" do
    get :edit, :id => upgrades(:one).to_param
    assert_response :success
  end

  test "should update upgrade" do
    put :update, :id => upgrades(:one).to_param, :upgrade => { }
    assert_redirected_to upgrade_path(assigns(:upgrade))
  end

  test "should destroy upgrade" do
    assert_difference('Upgrade.count', -1) do
      delete :destroy, :id => upgrades(:one).to_param
    end

    assert_redirected_to upgrades_path
  end
end
