-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Jan 13, 2026 at 01:00 PM
-- Server version: 10.4.28-MariaDB
-- PHP Version: 8.2.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `finance_db`
--

-- --------------------------------------------------------

--
-- Table structure for table `expenses`
--

CREATE TABLE `expenses` (
  `id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `category` varchar(100) NOT NULL,
  `amount` decimal(12,2) NOT NULL,
  `expense_date` date NOT NULL,
  `notes` text DEFAULT NULL,
  `created_at` timestamp NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `expenses`
--

INSERT INTO `expenses` (`id`, `user_id`, `category`, `amount`, `expense_date`, `notes`, `created_at`) VALUES
(1, 1, 'Food', 25.00, '2026-01-12', 'lunch', '2026-01-12 15:05:46'),
(2, 1, 'Transport', 10.00, '2026-01-12', 'Bus', '2026-01-12 15:06:02'),
(3, 1, 'Food', 10.00, '2026-01-12', 'dinner', '2026-01-12 15:09:58'),
(4, 6, 'Food', 25.00, '2026-01-13', 'lunch', '2026-01-13 03:45:14'),
(5, 1, 'Food', 3000.00, '2026-01-13', '', '2026-01-13 03:57:24'),
(6, 1, 'Entertainment', 4400.00, '2026-01-13', '', '2026-01-13 04:22:37');

-- --------------------------------------------------------

--
-- Table structure for table `income`
--

CREATE TABLE `income` (
  `id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `source` varchar(255) NOT NULL,
  `amount` decimal(10,2) NOT NULL,
  `income_date` date NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `income`
--

INSERT INTO `income` (`id`, `user_id`, `source`, `amount`, `income_date`) VALUES
(1, 1, 'Passive', 5500.00, '2026-01-12'),
(3, 1, 'Crypto', 2000.00, '2026-01-12'),
(4, 6, 'Passive', 10000.00, '2026-01-13');

-- --------------------------------------------------------

--
-- Table structure for table `reports`
--

CREATE TABLE `reports` (
  `id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `month` varchar(7) NOT NULL,
  `total_income` decimal(10,2) NOT NULL,
  `total_expense` decimal(10,2) NOT NULL,
  `balance` decimal(10,2) NOT NULL,
  `insights` longtext NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `reports`
--

INSERT INTO `reports` (`id`, `user_id`, `month`, `total_income`, `total_expense`, `balance`, `insights`, `created_at`) VALUES
(1, 1, '2026-01', 7500.00, 45.00, 7455.00, '👤 User: Sujay\n\n📊 Financial Summary\n------------------------------\n Income   : RM 7500.00\n Expenses : RM 45.00\n Balance  : RM 7455.00\n- Savings Rate: 99.4%\n\n------------------------------\n💳 Budget Status\n- Used: 15.0%\n✅ Budget under control\n\n📂 Spending Breakdown\n- Food: RM 35.0\n- Transport: RM 10.0\n\n📌 Highest Category: Food\n⚠️ High spending detected\n\n🎯 Saving Goal Progress\n- 100.0% achieved\n🎉 Goal achieved\n\n🤖 Smart Tips\n- Limit spending on Food\n- Track expenses weekly\n', '2026-01-12 15:12:09'),
(2, 1, '2026-01', 7500.00, 45.00, 7455.00, '👤 User: Sujay\n\n📊 Financial Summary\n------------------------------\n Income   : RM 7500.00\n Expenses : RM 45.00\n Balance  : RM 7455.00\n- Savings Rate: 99.4%\n\n------------------------------\n💳 Budget Status\n- Used: 15.0%\n✅ Budget under control\n\n📂 Spending Breakdown\n- Food: RM 35.0\n- Transport: RM 10.0\n\n📌 Highest Category: Food\n⚠️ High spending detected\n\n🎯 Saving Goal Progress\n- 100.0% achieved\n🎉 Goal achieved\n\n🤖 Smart Tips\n- Limit spending on Food\n- Track expenses regularly\n', '2026-01-13 01:00:44'),
(3, 6, '2026-01', 10000.00, 25.00, 9975.00, '👤 User: Sujay11\n\n📊 Financial Summary\n------------------------------\n Income   : RM 10000.00\n Expenses : RM 25.00\n Balance  : RM 9975.00\n- Savings Rate: 99.8%\n\n------------------------------\n💳 Budget Status\n- Used: 5.0%\n✅ Budget under control\n\n📂 Spending Breakdown\n- Food: RM 25.0\n\n📌 Highest Category: Food\n⚠️ High spending detected\n\n🎯 Saving Goal Progress\n- 100.0% achieved\n🎉 Goal achieved\n\n🤖 Smart Tips\n- Limit spending on Food\n- Track expenses regularly\n', '2026-01-13 03:51:56'),
(4, 1, '2026-01', 7500.00, 3045.00, 4455.00, '👤 User: Sujay\n\n📊 Financial Summary\n------------------------------\n Income   : RM 7500.00\n Expenses : RM 3045.00\n Balance  : RM 4455.00\n- Savings Rate: 59.4%\n\n------------------------------\n💳 Budget Status\n- Used: 1015.0%\n🚨 Budget exceeded\n📂 Spending Breakdown\n- Food: RM 3035.0\n- Transport: RM 10.0\n\n📌 Highest Category: Food\n⚠️ High spending detected\n\n🎯 Saving Goal Progress\n- 100.0% achieved\n🎉 Goal achieved\n\n🤖 Smart Tips\n- Limit spending on Food\n- Track expenses regularly\n', '2026-01-13 04:21:47'),
(5, 1, '2026-01', 7500.00, 7445.00, 55.00, '👤 User: Sujay\n\n📊 Financial Summary\n------------------------------\n Income   : RM 7500.00\n Expenses : RM 7445.00\n Balance  : RM 55.00\n- Savings Rate: 0.7%\n\n------------------------------\n💳 Budget Status\n- Used: 2481.7%\n🚨 Budget exceeded\n📂 Spending Breakdown\n- Entertainment: RM 4400.0\n- Food: RM 3035.0\n- Transport: RM 10.0\n\n📌 Highest Category: Entertainment\n⚠️ High spending detected\n\n🎯 Saving Goal Progress\n- 27.5% achieved\n💡 Keep saving consistently\n\n🤖 Smart Tips\n- Increase savings rate\n- Limit spending on Entertainment\n- Track expenses regularly\n', '2026-01-13 04:22:56');

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` int(11) NOT NULL,
  `username` varchar(100) NOT NULL,
  `password` varchar(255) NOT NULL,
  `monthly_budget` decimal(10,2) NOT NULL,
  `saving_goal` decimal(10,2) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`id`, `username`, `password`, `monthly_budget`, `saving_goal`) VALUES
(1, 'Sujay', '57eb66c7130867cac58c2e7a1fbb42b73cc2c3a5dfa0f870027334114573657f', 300.00, 200.00),
(2, 'Daarshana', 'b28d3c584a3b3054ee6a3d99586c1ee3617e4a0698825317a031904eb5d82f3e', 200.00, 100.00),
(3, 'Thanuesh', '52946e7ad82928fe1eb2342de3cde3e6bd64c3cf39b8b6be602b69baa35614bc', 1000.00, 500.00),
(4, 'Sujay1', '1d540061e3dbe9feccc943ff510998f7378c78ad7242a6058ebbab7205f5aa1a', 2000.00, 400.00),
(5, 'Sujay22', '9315d8e30f5ecc4fdce7f968d588bcc4f89479d80b66fd34fc963078c1d75547', 200.00, 1000.00),
(6, 'Sujay11', '5f7c1029fb099e6a9633d9da29c0e472731f0cfd2b5fd0768407199a1f9ef708', 500.00, 200.00);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `expenses`
--
ALTER TABLE `expenses`
  ADD PRIMARY KEY (`id`),
  ADD KEY `user_id` (`user_id`);

--
-- Indexes for table `income`
--
ALTER TABLE `income`
  ADD PRIMARY KEY (`id`),
  ADD KEY `user_id` (`user_id`);

--
-- Indexes for table `reports`
--
ALTER TABLE `reports`
  ADD PRIMARY KEY (`id`),
  ADD KEY `user_id` (`user_id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `expenses`
--
ALTER TABLE `expenses`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- AUTO_INCREMENT for table `income`
--
ALTER TABLE `income`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT for table `reports`
--
ALTER TABLE `reports`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `expenses`
--
ALTER TABLE `expenses`
  ADD CONSTRAINT `expenses_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `income`
--
ALTER TABLE `income`
  ADD CONSTRAINT `income_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `reports`
--
ALTER TABLE `reports`
  ADD CONSTRAINT `reports_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
