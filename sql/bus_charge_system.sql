-- 主机： 127.0.0.1:3306
-- 生成日期： 2021-12-12 21:10:21
-- 服务器版本： 8.0.18

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- 数据库： `bus_charge_system`
--
CREATE DATABASE IF NOT EXISTS `bus_charge_system` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;
USE `bus_charge_system`;

-- --------------------------------------------------------

--
-- 表的结构 `user_tb`
--

DROP TABLE IF EXISTS `user_tb`;
CREATE TABLE IF NOT EXISTS `user_tb` (
  `id` bigint(20) NOT NULL,
  `name` varchar(20) NOT NULL,
  `sex` char(2) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  `age` int(11) DEFAULT '0',
  `balance` double NOT NULL DEFAULT '0' COMMENT '用户余额',
  `signup_time` datetime NOT NULL COMMENT '注册时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- 转存表中的数据 `user_tb`
--

INSERT INTO `user_tb` (`id`, `name`, `sex`, `age`, `balance`, `signup_time`) VALUES
(2345, 'dxr', '男', 0, 30, '2021-12-12 08:28:49'),
(902210826, 'XMY', '男', 0, 7, '2021-12-10 09:06:57'),
(1705917121, 'FYQ', '男', 0, 308, '2021-12-10 09:08:16'),
(2178048141, 'ZMF', '男', 0, 6, '2021-12-10 09:09:37'),
(2513425333, 'FYQ', '男', 0, 8, '2021-12-10 09:07:09');
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
