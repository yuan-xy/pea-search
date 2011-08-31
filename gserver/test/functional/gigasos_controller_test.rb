require 'test_helper'

class GigasosControllerTest < ActionController::TestCase
  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:gigasos)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create gigaso" do
    assert_difference('Gigaso.count') do
      post :create, :gigaso => { }
    end

    assert_redirected_to gigaso_path(assigns(:gigaso))
  end

  test "should show gigaso" do
    get :show, :id => gigasos(:one).to_param
    assert_response :success
  end

  test "should get edit" do
    get :edit, :id => gigasos(:one).to_param
    assert_response :success
  end

  test "should update gigaso" do
    put :update, :id => gigasos(:one).to_param, :gigaso => { }
    assert_redirected_to gigaso_path(assigns(:gigaso))
  end

  test "should destroy gigaso" do
    assert_difference('Gigaso.count', -1) do
      delete :destroy, :id => gigasos(:one).to_param
    end

    assert_redirected_to gigasos_path
  end
end
