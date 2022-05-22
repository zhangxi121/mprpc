# aux_source_directory(. SRC_LIST)
set(SRC_LIST  mprpcapplication.cc  mprpcconfig.cc  rpcheader.pb.cc  
    rpcprovider.cc  mprpcchannel.cc  mprpccontroller.cc  logger.cc  
    zookeeperutil.cc)
# add_library(mprpc SHARED ${SRC_LIST})
add_library(mprpc ${SRC_LIST})
target_link_libraries(mprpc muduo_net  muduo_base  pthread  zookeeper_mt)   
# 必须是   muduo_net  muduo_base  pthread, 因为 muduo_base 依赖 muduo_net, 所以先得有 muduo_net, 再有 muduo_base,


