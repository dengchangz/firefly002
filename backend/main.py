"""
ZeroMQ 服务端 - 资金分析系统后端
"""
import zmq
import json
import logging
import time
import hashlib
import uuid
from threading import Thread
from typing import Callable, Dict, Any

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class ZmqServer:
    """ZeroMQ 服务端"""
    
    def __init__(self, req_port=5555, pub_port=5556):
        self.context = zmq.Context()
        
        # REP socket（处理请求）
        self.rep_socket = self.context.socket(zmq.REP)
        self.rep_socket.bind(f"tcp://*:{req_port}")
        
        # PUB socket（推送通知）
        self.pub_socket = self.context.socket(zmq.PUB)
        self.pub_socket.bind(f"tcp://*:{pub_port}")
        
        self.handlers: Dict[str, Callable] = {}
        self.running = False
        
        logger.info(f"ZeroMQ Server started on ports {req_port} (REP) and {pub_port} (PUB)")
    
    def register_handler(self, action: str, handler: Callable):
        """注册消息处理器"""
        self.handlers[action] = handler
        logger.info(f"Registered handler for action: {action}")
    
    def start(self):
        """启动服务"""
        self.running = True
        
        # 在独立线程中运行
        thread = Thread(target=self._run_loop, daemon=True)
        thread.start()
        
        logger.info("ZeroMQ Server is running...")
    
    def _run_loop(self):
        """消息处理循环"""
        while self.running:
            try:
                # 接收请求
                message = self.rep_socket.recv_string()
                logger.debug(f"Received message: {message[:100]}...")
                
                # 解析请求
                request = json.loads(message)
                action = request.get('action')
                params = request.get('params', {})
                msg_id = request.get('msg_id')
                
                # 调用处理器
                if action in self.handlers:
                    try:
                        result = self.handlers[action](params)
                        response = self._build_response(msg_id, 'success', result)
                    except Exception as e:
                        logger.error(f"Handler error: {e}", exc_info=True)
                        response = self._build_response(
                            msg_id, 'error', 
                            message=str(e), 
                            code=500
                        )
                else:
                    response = self._build_response(
                        msg_id, 'error',
                        message=f"Unknown action: {action}",
                        code=404
                    )
                
                # 发送响应
                self.rep_socket.send_string(json.dumps(response))
                
            except Exception as e:
                logger.error(f"Server error: {e}", exc_info=True)
                error_response = self._build_response(
                    None, 'error',
                    message="Internal server error",
                    code=500
                )
                self.rep_socket.send_string(json.dumps(error_response))
    
    def publish(self, notification_type: str, data: dict, topic: str = ""):
        """发布通知"""
        notification = {
            'type': notification_type,
            'data': data,
            'timestamp': int(time.time())
        }
        
        message = f"{topic}:{json.dumps(notification)}" if topic else json.dumps(notification)
        self.pub_socket.send_string(message)
        logger.debug(f"Published notification: {notification_type}")
    
    def _build_response(self, msg_id, status, data=None, message="", code=200):
        """构建响应消息"""
        return {
            'msg_id': msg_id,
            'timestamp': int(time.time()),
            'status': status,
            'code': code,
            'data': data or {},
            'message': message,
            'error': None if status == 'success' else message
        }
    
    def stop(self):
        """停止服务"""
        self.running = False
        self.rep_socket.close()
        self.pub_socket.close()
        self.context.term()
        logger.info("ZeroMQ Server stopped")


# 测试处理器
def handle_ping(params: dict) -> dict:
    """测试Ping处理器"""
    logger.info("Handling ping request")
    return {
        'message': 'pong',
        'timestamp': int(time.time()),
        'echo': params
    }


def handle_task_create(params: dict) -> dict:
    """创建任务处理器"""
    logger.info(f"Creating task: {params}")
    
    task_name = params.get('task_name', 'New Task')
    
    # TODO: 实际的任务创建逻辑
    task_id = f"task_{int(time.time())}"
    
    return {
        'task_id': task_id,
        'task_name': task_name,
        'created_at': int(time.time())
    }


def handle_task_list(params: dict) -> dict:
    """获取任务列表处理器"""
    logger.info("Listing tasks")
    
    # TODO: 从数据库查询任务列表
    tasks = [
        {
            'task_id': 'task_001',
            'task_name': '示例任务1',
            'created_at': int(time.time()) - 86400
        },
        {
            'task_id': 'task_002',
            'task_name': '示例任务2',
            'created_at': int(time.time()) - 3600
        }
    ]
    
    return {
        'tasks': tasks,
        'total': len(tasks)
    }


# 用户认证处理器

# 模拟用户数据库（实际应用中应该使用数据库）
USERS_DB = {
    'admin': {
        'password_hash': hashlib.sha256('admin123'.encode()).hexdigest(),
        'role': 'administrator',
        'permissions': ['*']  # 所有权限
    },
    'user': {
        'password_hash': hashlib.sha256('user123'.encode()).hexdigest(),
        'role': 'user',
        'permissions': ['data.import', 'data.query', 'analysis.execute', 'report.generate']
    },
    'guest': {
        'password_hash': hashlib.sha256('guest123'.encode()).hexdigest(),
        'role': 'guest',
        'permissions': ['data.query']
    }
}

# 会话存储（实际应用中应该使用Redis等）
SESSIONS = {}


def handle_auth_login(params: dict) -> dict:
    """用户登录处理器"""
    username = params.get('username', '').strip()
    password_hash = params.get('password', '')
    
    logger.info(f"Login attempt for user: {username}")
    
    # 验证用户名
    if not username or username not in USERS_DB:
        logger.warning(f"Login failed: user not found - {username}")
        raise Exception("用户名或密码错误")
    
    user_data = USERS_DB[username]
    
    # 验证密码
    if password_hash != user_data['password_hash']:
        logger.warning(f"Login failed: wrong password - {username}")
        raise Exception("用户名或密码错误")
    
    # 生成会话令牌
    session_token = str(uuid.uuid4())
    
    # 保存会话
    SESSIONS[session_token] = {
        'username': username,
        'role': user_data['role'],
        'permissions': user_data['permissions'],
        'login_time': int(time.time()),
        'last_activity': int(time.time())
    }
    
    logger.info(f"Login successful: {username} (role: {user_data['role']})")
    
    return {
        'success': True,
        'session_token': session_token,
        'username': username,
        'role': user_data['role'],
        'permissions': user_data['permissions'],
        'login_time': int(time.time())
    }


def handle_auth_logout(params: dict) -> dict:
    """用户登出处理器"""
    session_token = params.get('session_token', '')
    
    if session_token in SESSIONS:
        username = SESSIONS[session_token]['username']
        del SESSIONS[session_token]
        logger.info(f"User logged out: {username}")
        return {'success': True, 'message': '登出成功'}
    
    return {'success': False, 'message': '会话不存在'}


def handle_auth_verify(params: dict) -> dict:
    """验证会话处理器"""
    session_token = params.get('session_token', '')
    
    if session_token not in SESSIONS:
        raise Exception("会话已过期或无效")
    
    session = SESSIONS[session_token]
    
    # 更新最后活动时间
    session['last_activity'] = int(time.time())
    
    # 检查会话是否超时（例如 24 小时）
    if int(time.time()) - session['login_time'] > 86400:
        del SESSIONS[session_token]
        raise Exception("会话已过期")
    
    return {
        'valid': True,
        'username': session['username'],
        'role': session['role'],
        'permissions': session['permissions']
    }


def main():
    """主函数"""
    print("=" * 60)
    print("  资金分析系统 - Python后端服务")
    print("  Version: 1.0.0")
    print("=" * 60)
    
    # 创建服务器
    server = ZmqServer(req_port=5555, pub_port=5556)
    
    # 注册处理器
    server.register_handler('test.ping', handle_ping)
    server.register_handler('task.create', handle_task_create)
    server.register_handler('task.list', handle_task_list)
    
    # 注册认证处理器
    server.register_handler('auth.login', handle_auth_login)
    server.register_handler('auth.logout', handle_auth_logout)
    server.register_handler('auth.verify', handle_auth_verify)
    
    # 启动服务
    server.start()
    
    print("\n服务已启动:")
    print("  REQ-REP: tcp://*:5555")
    print("  PUB-SUB: tcp://*:5556")
    print("\n按 Ctrl+C 停止服务\n")
    
    # 测试：定期发送通知
    def send_test_notifications():
        import time
        count = 0
        while True:
            time.sleep(10)
            count += 1
            server.publish('test', {
                'message': f'Test notification #{count}',
                'timestamp': int(time.time())
            })
    
    # 启动通知线程
    notification_thread = Thread(target=send_test_notifications, daemon=True)
    notification_thread.start()
    
    # 保持运行
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n正在关闭服务...")
        server.stop()
        print("服务已停止")


if __name__ == '__main__':
    main()
