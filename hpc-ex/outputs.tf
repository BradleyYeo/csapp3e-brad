output "instance_id" {
  description = "EC2 Instance ID of the OpenCHAMI Head Node"
  value       = aws_instance.head_node.id
}

output "public_ip" {
  description = "Public IPv4 address of the OpenCHAMI Head Node"
  value       = aws_instance.head_node.public_ip
}

output "private_ip" {
  description = "Private IPv4 address of the OpenCHAMI Head Node"
  value       = aws_instance.head_node.private_ip
}

output "private_key_path" {
  description = "Local path to the generated SSH private key"
  value       = local_sensitive_file.private_key.filename
}

output "ssh_command" {
  description = "Command to SSH directly into the OpenCHAMI Head Node"
  value       = "ssh -i ${local_sensitive_file.private_key.filename} rocky@${aws_instance.head_node.public_ip}"
}

output "resolved_ami_id" {
  description = "Resolved Rocky Linux 9 AMI ID deployed"
  value       = local.resolved_ami_id
}

output "worker_instances" {
  description = "Worker nodes detailed network information"
  value = [
    for i in range(var.worker_count) : {
      name        = "openchami-worker-${i + 1}"
      id          = aws_instance.worker[i].id
      subnet_id   = aws_subnet.worker[i].id
      subnet_cidr = aws_subnet.worker[i].cidr_block
      az          = aws_subnet.worker[i].availability_zone
      public_ip   = aws_instance.worker[i].public_ip
      private_ip  = aws_instance.worker[i].private_ip
      ssh_command = "ssh -i ${local_sensitive_file.private_key.filename} rocky@${aws_instance.worker[i].public_ip}"
    }
  ]
}

output "cluster_inventory" {
  description = "Cluster private IP inventory for /etc/hosts or ansible"
  value = concat(
    ["${aws_instance.head_node.private_ip} head"],
    [for i in range(var.worker_count) : "${aws_instance.worker[i].private_ip} worker-${i + 1}"]
  )
}

