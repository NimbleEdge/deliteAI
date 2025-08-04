/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import UIKit
import DeliteAI
import SwiftProtobuf

class ChatViewController: UIViewController, UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate {

    private var messages: [(String, Bool)] = []
    private var isModelActive = false

    private let tableView: UITableView = {
        let tableView = UITableView()
        tableView.translatesAutoresizingMaskIntoConstraints = false
        tableView.separatorStyle = .none
        tableView.register(MessageCell.self, forCellReuseIdentifier: "MessageCell")
        return tableView
    }()

    private let messageInputViewContainer: UIView = {
        let view = UIView()
        view.translatesAutoresizingMaskIntoConstraints = false
        view.backgroundColor = .secondarySystemBackground
        return view
    }()

    private let inputTextField: UITextField = {
        let textField = UITextField()
        textField.translatesAutoresizingMaskIntoConstraints = false
        textField.placeholder = "Type your prompt..."
        
        // Custom look
        textField.borderStyle = .none
        textField.backgroundColor = .secondarySystemBackground
        textField.layer.cornerRadius = 16
        textField.layer.masksToBounds = true
        textField.layer.borderWidth = 1
        textField.layer.borderColor = UIColor.systemGray2.cgColor

        // Padding using left/right views (without extensions)
        let leftPadding = UIView(frame: CGRect(x: 0, y: 0, width: 12, height: 0))
        textField.leftView = leftPadding
        textField.leftViewMode = .always

        let rightPadding = UIView(frame: CGRect(x: 0, y: 0, width: 12, height: 0))
        textField.rightView = rightPadding
        textField.rightViewMode = .always

        return textField
    }()

    private let sendButton: UIButton = {
        let button = UIButton(type: .system)
        button.translatesAutoresizingMaskIntoConstraints = false
        button.setTitle("Send", for: .normal)
        return button
    }()
    
    private let loadingOverlay: UIView = {
        let overlay = UIView()
        overlay.translatesAutoresizingMaskIntoConstraints = false
        overlay.backgroundColor = UIColor.black.withAlphaComponent(0.5)

        let spinner = UIActivityIndicatorView(style: .large)
        spinner.color = .white
        spinner.translatesAutoresizingMaskIntoConstraints = false
        spinner.startAnimating()

        let label = UILabel()
        label.translatesAutoresizingMaskIntoConstraints = false
        label.text = "Getting things ready..."
        label.textColor = .white
        label.font = .systemFont(ofSize: 16, weight: .medium)

        overlay.addSubview(spinner)
        overlay.addSubview(label)

        NSLayoutConstraint.activate([
            spinner.centerXAnchor.constraint(equalTo: overlay.centerXAnchor),
            spinner.centerYAnchor.constraint(equalTo: overlay.centerYAnchor, constant: -12),

            label.topAnchor.constraint(equalTo: spinner.bottomAnchor, constant: 12),
            label.centerXAnchor.constraint(equalTo: overlay.centerXAnchor)
        ])

        return overlay
    }()
    
    private let scrollToBottomButton: UIButton = {
        let button = UIButton(type: .system)
        button.translatesAutoresizingMaskIntoConstraints = false
        let config = UIImage.SymbolConfiguration(pointSize: 14, weight: .bold)
        let arrowImage = UIImage(systemName: "arrow.down", withConfiguration: config)
        button.setImage(arrowImage, for: .normal)
        button.tintColor = .white
        button.backgroundColor = UIColor.systemBlue
        button.layer.cornerRadius = 15
        button.clipsToBounds = true
        button.isHidden = true
        return button
    }()



    override func viewDidLoad() {
        super.viewDidLoad()

        setupStyle()
        setupLayout()
        setupDelegates()
        setupActions()
        initializeNimbleNet()
    }
    
    private func setupStyle() {
        
        view.backgroundColor = .systemBackground
        title = "DeliteAI Example"
        
        let appearance = UINavigationBarAppearance()
        appearance.configureWithOpaqueBackground()
        appearance.backgroundColor = .secondarySystemBackground
        navigationController?.navigationBar.standardAppearance = appearance
        navigationController?.navigationBar.scrollEdgeAppearance = appearance
        navigationController?.navigationBar.tintColor = .white
        
    }

    private func setupLayout() {
        view.addSubview(tableView)
        view.addSubview(messageInputViewContainer)
        view.addSubview(loadingOverlay)
        messageInputViewContainer.addSubview(inputTextField)
        messageInputViewContainer.addSubview(sendButton)
        view.addSubview(scrollToBottomButton)


        NSLayoutConstraint.activate([
            // Table view
            tableView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            tableView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            tableView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            tableView.bottomAnchor.constraint(equalTo: messageInputViewContainer.topAnchor),

            // Input container
            messageInputViewContainer.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            messageInputViewContainer.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            messageInputViewContainer.bottomAnchor.constraint(equalTo: view.bottomAnchor),

            inputTextField.leadingAnchor.constraint(equalTo: messageInputViewContainer.leadingAnchor, constant: 15),
            inputTextField.topAnchor.constraint(equalTo: messageInputViewContainer.topAnchor, constant: 8),
            inputTextField.bottomAnchor.constraint(equalTo: messageInputViewContainer.bottomAnchor, constant: -40),
            inputTextField.heightAnchor.constraint(equalToConstant: 40),

            sendButton.leadingAnchor.constraint(equalTo: inputTextField.trailingAnchor, constant: 8),
            sendButton.trailingAnchor.constraint(equalTo: messageInputViewContainer.trailingAnchor, constant: -8),
            sendButton.centerYAnchor.constraint(equalTo: inputTextField.centerYAnchor),
            sendButton.widthAnchor.constraint(equalToConstant: 60),
            
            loadingOverlay.topAnchor.constraint(equalTo: view.topAnchor),
            loadingOverlay.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            loadingOverlay.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            loadingOverlay.bottomAnchor.constraint(equalTo: view.bottomAnchor),
            
            scrollToBottomButton.centerXAnchor.constraint(equalTo: view.centerXAnchor),
            scrollToBottomButton.bottomAnchor.constraint(equalTo: messageInputViewContainer.topAnchor, constant: -10),
            scrollToBottomButton.widthAnchor.constraint(equalToConstant: 30),
            scrollToBottomButton.heightAnchor.constraint(equalToConstant: 30)
        ])
    }

    private func setupDelegates() {
        tableView.delegate = self
        tableView.dataSource = self
        inputTextField.delegate = self
    }

    private func setupActions() {
        sendButton.addTarget(self, action: #selector(sendTapped), for: .touchUpInside)
        scrollToBottomButton.addTarget(self, action: #selector(scrollToBottomTapped), for: .touchUpInside)
    }

    @objc private func sendTapped() {
        
        if isModelActive {
            
            DispatchQueue.global().async {
                let response = self.stopLLM()
                DispatchQueue.main.async {
                    self.sendButton.setTitle("Send", for: .normal)
                }
            }
            
        } else {
            
            guard let text = inputTextField.text?.trimmingCharacters(in: .whitespacesAndNewlines), !text.isEmpty else { return }
            inputTextField.text = ""
            addMessage(text, isUser: true)
            sendButton.setTitle("Stop", for: .normal)
            
            DispatchQueue.global().async {
                let response = self.callModelStreaming(prompt: text)
                DispatchQueue.main.async {
                    self.sendButton.setTitle("Send", for: .normal)
                }
            }
            
        }

    }

    private func addMessage(_ text: String, isUser: Bool) {
        DispatchQueue.main.async {
            self.messages.append((text, isUser))
            self.tableView.reloadData()
            self.scrollToBottom()
        }
    }

    private func updateLastBotMessage(_ text: String) {
        DispatchQueue.main.async {
            if let last = self.messages.last, !last.1 {
                self.messages[self.messages.count - 1].0 = text
                self.tableView.reloadRows(at: [IndexPath(row: self.messages.count - 1, section: 0)], with: .none)
                self.checkScrollPosition()
            } else {
                self.messages.append((text, false))
                self.tableView.reloadData()
                self.scrollToBottom()
            }
        }
    }

    private func scrollToBottom() {
        let indexPath = IndexPath(row: max(0, messages.count - 1), section: 0)
        tableView.scrollToRow(at: indexPath, at: .bottom, animated: true)
    }

    // MARK: - NimbleNet
    private func initializeNimbleNet() {
        let config = NimbleNetConfig(
            clientId: "chatapp-test",
            clientSecret: BundleConfig.clientSecret,
            host: BundleConfig.host,
            deviceId: "hello-ios",
            debug: true,
            compatibilityTag: "EXECUTORCH_HF",
            online: true
        )
        
        let initialized = NimbleNetApi.initialize(config: config)
        print("isInitialized? \(initialized)")

        DispatchQueue.global().async {
            while !NimbleNetApi.isReady().status {
                print("Waiting for model to be ready...")
                Thread.sleep(forTimeInterval: 1)
            }
            DispatchQueue.main.async {
                self.loadingOverlay.removeFromSuperview()
                print("Model is ready")
            }
        }
    }



    // MARK: - TableView
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        messages.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        guard let cell = tableView.dequeueReusableCell(withIdentifier: "MessageCell", for: indexPath) as? MessageCell else {
            return UITableViewCell()
        }

        let (text, isUser) = messages[indexPath.row]
        cell.configure(with: text, isUser: isUser)
        return cell
    }
    
    func scrollViewDidScroll(_ scrollView: UIScrollView) {
        checkScrollPosition()
    }
    
    private func checkScrollPosition() {
        scrollToBottomButton.isHidden = isTableViewAtBottom()
    }
    
    private func isTableViewAtBottom() -> Bool {
        let offset = tableView.contentOffset.y
        let contentHeight = tableView.contentSize.height
        let visibleHeight = tableView.frame.size.height
        return offset >= contentHeight - visibleHeight - 10
    }
    
    @objc private func scrollToBottomTapped() {
        scrollToBottom(animated: true)
    }
        
    private func scrollToBottom(animated: Bool) {
        let indexPath = IndexPath(row: max(0, messages.count - 1), section: 0)
        tableView.scrollToRow(at: indexPath, at: .bottom, animated: animated)
    }

}

// MARK: - Model Interactors
extension ChatViewController {
    
    private func stopLLM() {
        isModelActive = false
        let _ = NimbleNetApi.runMethod(methodName: "stop_running", inputs: [:])
    }
    
    private func clearPrompt() {
        let input: [String: NimbleNetTensor] = [
            "context": NimbleNetTensor(data: [Int32](), datatype: .int32, shape: [0])
        ]
        _ = NimbleNetApi.runMethod(methodName: "clear_prompt", inputs: input)
    }
    
    private func callModelStreaming(prompt: String) -> String {
        let modelInputs: [String: NimbleNetTensor] = [
            "query": NimbleNetTensor(data: prompt, datatype: .string, shape: nil)
        ]
        _ = NimbleNetApi.runMethod(methodName: "prompt_llm", inputs: modelInputs)

        var outputString = ""
        var tokenBuffer = ""
        isModelActive = true

        while true {
            let response = NimbleNetApi.runMethod(methodName: "get_next_str", inputs: [:])
            guard let payload = response.payload else {
                print("No payload")
                isModelActive = false
                break
            }

            var outputMap: [String: NimbleNetTensor] = [:]
            for (key, value) in payload.map {
                outputMap[key] = NimbleNetTensor(data: value.data, datatype: value.type, shape: value.shape)
            }

            if outputMap["finished"] != nil {
                isModelActive = false
                break
            }

            if let str = outputMap["str"]?.data as? String {
                tokenBuffer += str
                if tokenBuffer.components(separatedBy: .whitespaces).count >= 3 {
                    outputString += tokenBuffer
                    tokenBuffer = ""
                    self.updateLastBotMessage(outputString)
                }
            }
            
            if !isModelActive { break }

            Thread.sleep(forTimeInterval: 0.2)
        }

        if !tokenBuffer.isEmpty {
            outputString += tokenBuffer
            self.updateLastBotMessage(outputString)
        }

        return outputString
    }
    
}
