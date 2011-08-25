require 'test_helper'

class DumpsControllerTest < ActionController::TestCase
  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:dumps)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create dump" do
    assert_difference('Dump.count') do
      post :create, :dump => { }
    end

    assert_redirected_to dump_path(assigns(:dump))
  end

  test "should show dump" do
    get :show, :id => dumps(:one).to_param
    assert_response :success
  end

  test "should get edit" do
    get :edit, :id => dumps(:one).to_param
    assert_response :success
  end

  test "should update dump" do
    put :update, :id => dumps(:one).to_param, :dump => { }
    assert_redirected_to dump_path(assigns(:dump))
  end

  test "should destroy dump" do
    assert_difference('Dump.count', -1) do
      delete :destroy, :id => dumps(:one).to_param
    end

    assert_redirected_to dumps_path
  end
end
